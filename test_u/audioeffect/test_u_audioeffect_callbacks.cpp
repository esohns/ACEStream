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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "Dmo.h"
#include "functiondiscoverykeys_devpkey.h"
#include "mfapi.h"
#include "mferror.h"
#undef GetObject
#include "mfidl.h"
#include "mfreadwrite.h"
#include "mmdeviceapi.h"
#include "olectl.h"
#include "OleAuto.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#include "wmcodecdsp.h"
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}
#endif // ACE_WIN32 || ACE_WIN64

#if defined (LIBNOISE_SUPPORT)
#include "noise/noise.h"
#endif // LIBNOISE_SUPPORT

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

#include <limits>
#include <map>
#include <set>
#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Dirent_Selector.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#include "common_file_tools.h"
#include "common_os_tools.h"
#include "common_process_tools.h"

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
#include "stream_lib_directsound_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#include "stream_lib_tools.h"
#else
#include "stream_lib_alsa_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_common_modules.h"
#include "test_u_audioeffect_defines.h"
#include "test_u_audioeffect_gl_callbacks.h"
#include "test_u_audioeffect_stream.h"

// global variables
bool untoggling_record_button = false;

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_capture_devices (GtkListStore* listStore_in,
                      enum Stream_Device_Capturer capturer_in)
#else
load_capture_devices (GtkListStore* listStore_in,
                      const std::string& deviceIdentifier_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::load_capture_devices"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  std::string device_identifier_string;
  std::string device_friendlyname_string;
  UINT device_id_i = 0;
  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_WAVEIN:
    {
      MMRESULT result_3 = MMSYSERR_NOERROR;
      WAVEINCAPS capabilities_s;
      UINT num_devices_i = waveInGetNumDevs ();
      for (UINT i = 0;
           i < num_devices_i;
           ++i)
      {
        result_3 = waveInGetDevCaps (i,
                                     &capabilities_s,
                                     sizeof (WAVEINCAPS));
        if (unlikely (result_3 != MMSYSERR_NOERROR))
        { char error_msg_a[BUFSIZ];
          waveInGetErrorText (result_3, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to waveInGetDevCaps(%d): \"%s\", aborting\n"),
                      i, ACE_TEXT (error_msg_a)));
          return false;
        } // end IF
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\": [manufacturer id,product id]: %d/%d; [driver version]: %d.%d\n"),
        //            ACE_TEXT (capabilities_s.szPname),
        //            capabilities_s.wMid, capabilities_s.wPid,
        //            (capabilities_s.vDriverVersion & 0xFFFF0000) >> 16, (capabilities_s.vDriverVersion & 0xFFFF)));

        device_identifier_string =
          Common_OS_Tools::GUIDToString (Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (i,
                                                                                                                 true));
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, ACE_TEXT_ALWAYS_CHAR (capabilities_s.szPname),
                            1, device_identifier_string.c_str (),
                            2, i,
                            -1);
      } // end FOR

      result = true;
      break;
    }
    case STREAM_DEVICE_CAPTURER_WASAPI:
    {
      IMMDeviceEnumerator* enumerator_p = NULL;
      result_2 =
        CoCreateInstance (__uuidof (MMDeviceEnumerator), NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS (&enumerator_p));
      ACE_ASSERT (SUCCEEDED (result_2) && enumerator_p);
      IMMDeviceCollection* devices_p = NULL;
      result_2 = enumerator_p->EnumAudioEndpoints (eCapture,
                                                   DEVICE_STATE_ACTIVE,
                                                   &devices_p);
      ACE_ASSERT (SUCCEEDED (result_2) && devices_p);
      enumerator_p->Release (); enumerator_p = NULL;
      UINT num_devices_i = 0;
      result_2 = devices_p->GetCount (&num_devices_i);
      ACE_ASSERT (SUCCEEDED (result_2));
      IMMDevice* device_p = NULL;
      struct _GUID GUID_s = GUID_NULL;
      IPropertyStore* property_store_p = NULL;
      struct tagPROPVARIANT property_s;
      PropVariantInit (&property_s);
      for (UINT i = 0;
           i < num_devices_i;
           ++i)
      { ACE_ASSERT (!device_p);
        result_2 = devices_p->Item (i,
                                    &device_p);
        ACE_ASSERT (SUCCEEDED (result_2) && device_p);
        result_2 = device_p->OpenPropertyStore (STGM_READ,
                                                &property_store_p);
        ACE_ASSERT (SUCCEEDED (result_2) && property_store_p);
        device_p->Release (); device_p = NULL;

        result_2 = property_store_p->GetValue (PKEY_Device_FriendlyName,
                                               &property_s);
        ACE_ASSERT (SUCCEEDED (result_2));
        ACE_ASSERT (property_s.vt == VT_LPWSTR);
        device_friendlyname_string =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (property_s.pwszVal));
        PropVariantClear (&property_s);
        result_2 = property_store_p->GetValue (PKEY_AudioEndpoint_GUID,
                                               &property_s);
        ACE_ASSERT (SUCCEEDED (result_2));
        property_store_p->Release (); property_store_p = NULL;
        ACE_ASSERT (property_s.vt == VT_LPWSTR);
        device_identifier_string =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (property_s.pwszVal));
        PropVariantClear (&property_s);
        GUID_s = Common_OS_Tools::StringToGUID (device_identifier_string);
        device_id_i =
          Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (GUID_s);

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, device_friendlyname_string.c_str (),
                            1, device_identifier_string.c_str (),
                            2, device_id_i,
                            -1);
      } // end FOR
      devices_p->Release (); devices_p = NULL;

      result = true;
      break;
    }
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
    {
      ICreateDevEnum* enumerator_p = NULL;
      IEnumMoniker* enum_moniker_p = NULL;
      IMoniker* moniker_p = NULL;
      IPropertyBag* properties_p = NULL;
      struct tagVARIANT variant_s;
      std::string description_string, device_path;

      result_2 =
        CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS (&enumerator_p));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        goto error;
      } // end IF
      ACE_ASSERT (enum_moniker_p);
      enumerator_p->Release (); enumerator_p = NULL;

      while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
      { ACE_ASSERT (moniker_p);
        properties_p = NULL;
        result_2 = moniker_p->BindToStorage (NULL, NULL,
                                             IID_PPV_ARGS (&properties_p));
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
          goto error;
        } // end IF
        moniker_p->Release (); moniker_p = NULL;
        ACE_ASSERT (properties_p);

        VariantInit (&variant_s);
        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_NAME_STRING_L,
                              &variant_s,
                              0);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_NAME_STRING_L),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
          goto error;
        } // end IF
        device_friendlyname_string =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant_s.bstrVal));
        VariantClear (&variant_s);
        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING_L,
                              &variant_s,
                              0);
        if (SUCCEEDED (result_2))
        {
          description_string =
            ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant_s.bstrVal));
          VariantClear (&variant_s);
        } // end IF
        else // 0x80070002 : ERROR_FILE_NOT_FOUND
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING_L),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));

        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_PATH_STRING_L,
                              &variant_s,
                              0);
        if (SUCCEEDED (result_2))
        {
          device_path =
            ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant_s.bstrVal));
          VariantClear (&variant_s);
        } // end IF
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_PATH_STRING_L),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));

        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_ID_STRING_L,
                              &variant_s,
                              0);
        if (SUCCEEDED (result_2))
        {
          device_id_i = variant_s.lVal;
          VariantClear (&variant_s);
        } // end IF
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_ID_STRING_L),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        properties_p->Release (); properties_p = NULL;
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\": \"%s\" @ %s (id: %d)\n"),
        //            ACE_TEXT (device_friendlyname_string.c_str ()),
        //            ACE_TEXT (description_string.c_str ()),
        //            ACE_TEXT (device_path.c_str ()),
        //            device_id_i));

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, device_friendlyname_string.c_str (),
                            1, device_path.c_str (),
                            2, device_id_i,
                            -1);
      } // end WHILE
      enum_moniker_p->Release (); enum_moniker_p = NULL;

      result = true;
      break;

error:
      if (enumerator_p)
        enumerator_p->Release ();
      if (enum_moniker_p)
        enum_moniker_p->Release ();
      if (moniker_p)
        moniker_p->Release ();
      if (properties_p)
        properties_p->Release ();
      VariantClear (&variant_s);
      break;
    }
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    {
      IMFAttributes* attributes_p = NULL;
      IMFActivate** devices_pp = NULL;
      UINT32 item_count = 0;
      WCHAR buffer_a[BUFSIZ];
      UINT32 length = 0;
      std::string device_endpointid_string;

      result_2 = MFCreateAttributes (&attributes_p, 1);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        goto error_2;
      } // end IF
      result_2 = MFEnumDeviceSources (attributes_p,
                                      &devices_pp,
                                      &item_count);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
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
        device_friendlyname_string =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));
        length = 0;
        result_2 =
          devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID,
                                        buffer_a,
                                        sizeof (WCHAR[BUFSIZ]),
                                        &length);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error_2;
        } // end IF
        device_endpointid_string =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));
        device_id_i =
          Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (Stream_MediaFramework_DirectSound_Tools::endpointIdToDirectSoundGUID (device_endpointid_string));
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\": \"%s\"\n"),
        //            ACE_TEXT (device_friendlyname_string.c_str ()),
        //            ACE_TEXT (device_endpointid_string.c_str ())));

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, device_friendlyname_string.c_str (),
                            1, device_endpointid_string.c_str (),
                            2, device_id_i,
                            -1);
      } // end FOR

      for (UINT32 i = 0; i < item_count; i++)
        devices_pp[i]->Release ();
      CoTaskMemFree (devices_pp); devices_pp = NULL;

      result = true;
      break;

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
                  ACE_TEXT ("invalid/unknown capturer type (was: %d), aborting\n"),
                  capturer_in));
      return false;
    }
  } // end SWITCH
#else
  int card_i = -1;
  int result_2 = -1;
  char* string_p = NULL, *string_2 = NULL;
  void** hints_p = NULL;
  std::string device_name_string;

  if (!deviceIdentifier_in.empty ())
  {
    card_i = Stream_MediaFramework_ALSA_Tools::getCardNumber (deviceIdentifier_in);
    if (unlikely (card_i == -1))
    { // *NOTE*: might be a virtual device like "default"
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getCardNumber(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceIdentifier_in.c_str ())));
      return false;
    } // end IF
    result_2 = snd_card_get_longname (card_i, &string_p);
    if (result_2 || !string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_card_get_longname(%d): \"%s\", aborting\n"),
                  card_i,
                  ACE_TEXT (snd_strerror (result_2))));
      return false;
    } // end IF
    result_2 =
        snd_device_name_hint (card_i,
                              ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_PCM_INTERFACE_NAME),
                              &hints_p);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_hint(%d): \"%s\", aborting\n"),
                  card_i,
                  ACE_TEXT (snd_strerror (result_2))));
      free (string_p);
      return false;
    } // end IF
    for (void** i = hints_p;
         *i;
         ++i)
    {
      string_2 = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("IOID"));
      if (!string_2) // NULL --> both
        goto continue_;
      if (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("Input")))
      {
        free (string_2); string_2 = NULL;
        continue;
      } // end IF
      free (string_2); string_2 = NULL;
continue_:
      string_2 = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("NAME"));
      if (!string_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_device_name_get_hint(NAME): \"%m\", continuing\n")));
        continue;
      } // end IF
      if (//(ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("default")) == 0)             ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("dmix:"), 5) == 0)           ||
          //(ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("dsnoop:"), 7) == 0)         ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("front:"), 6) == 0)          ||
          (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("null")) == 0)                ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("plughw:"), 7) == 0)         ||
          //(ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("pipewire")) == 0)            ||
          (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("pulse")) == 0)               ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround21:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround40:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround41:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround50:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround51:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround71:"), 11) == 0))
      {
        free (string_2); string_2 = NULL;
        continue;
      } // end IF
      device_name_string = string_2;
      // std::string::size_type position_i = device_name_string.find (':', 0);
      // if (position_i != std::string::npos)
      //   device_name_string.erase (position_i, std::string::npos);
      free (string_2); string_2 = NULL;
      string_2 = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("DESC"));
      if (!string_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_device_name_get_hint(DESC): \"%m\", continuing\n")));
        continue;
      } // end IF

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("found device \"%s\"/\"%s\": \"%s\"\n"),
                  ACE_TEXT (string_p),
                  ACE_TEXT (device_name_string.c_str ()),
                  ACE_TEXT (string_2)));

      gtk_list_store_append (listStore_in, &iterator);
      gtk_list_store_set (listStore_in, &iterator,
                          0, ACE_TEXT_ALWAYS_CHAR (string_2),
                          1, ACE_TEXT_ALWAYS_CHAR (device_name_string.c_str ()),
                          2, card_i,
                          -1);

      free (string_2); string_2 = NULL;
      snd_device_name_free_hint (hints_p); hints_p = NULL;
      free (string_p); string_p = NULL;

      break;
    } // end FOR

    return true;
  } // end IF

  do
  {
    result_2 = snd_card_next (&card_i);
    if (result_2 || (card_i == -1))
      break;
    if (snd_card_get_longname (card_i, &string_p) || !string_p)
      continue;
    result_2 =
        snd_device_name_hint (card_i,
                              ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_PCM_INTERFACE_NAME),
                              &hints_p);
    if (result_2 < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_hint(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result_2))));
      free (string_p); string_p = NULL;
      continue;
    } // end IF
    for (void** i = hints_p;
         *i;
         ++i)
    {
      string_2 = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("IOID"));
      if (!string_2) // NULL --> both
        goto continue_2;
      if (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("Input")))
      {
        free (string_2); string_2 = NULL;
        continue;
      } // end IF
      free (string_2); string_2 = NULL;
continue_2:
      string_2 = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("NAME"));
      if (!string_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", continuing\n")));
        continue;
      } // end IF
      if (//(ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("default")) == 0)             ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("dmix:"), 5) == 0)           ||
          //(ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("dsnoop:"), 7) == 0)         ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("front:"), 6) == 0)          ||
          (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("null")) == 0)                ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("plughw:"), 7) == 0)         ||
          //(ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("pipewire")) == 0)            ||
          (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("pulse")) == 0)               ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround21:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround40:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround41:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround50:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround51:"), 11) == 0)    ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("surround71:"), 11) == 0))
      {
        free (string_2); string_2 = NULL;
        continue;
      } // end IF
      device_name_string = string_2;
      // std::string::size_type position_i = device_name_string.find (':', 0);
      // if (position_i != std::string::npos)
      //   device_name_string.erase (position_i, std::string::npos);
      free (string_2); string_2 = NULL;
      string_2 = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("DESC"));
      if (!string_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", continuing\n")));
        continue;
      } // end IF

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("found device \"%s\"/\"%s\": \"%s\"\n"),
                  ACE_TEXT (string_p),
                  ACE_TEXT (device_name_string.c_str ()),
                  ACE_TEXT (string_2)));

      gtk_list_store_append (listStore_in, &iterator);
      gtk_list_store_set (listStore_in, &iterator,
                          0, ACE_TEXT_ALWAYS_CHAR (string_2),
                          1, ACE_TEXT_ALWAYS_CHAR (device_name_string.c_str ()),
                          2, card_i,
                          -1);

      free (string_2); string_2 = NULL;
    } // end FOR
    snd_device_name_free_hint (hints_p); hints_p = NULL;
    free (string_p); string_p = NULL;
  } while (true);

  result = true;
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

bool
load_all_formats (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_all_formats"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  gtk_list_store_append (listStore_in, &iterator);
  gtk_list_store_set (listStore_in, &iterator,
                      0, Stream_MediaFramework_Tools::mediaSubTypeToString (MEDIASUBTYPE_PCM, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str (),
                      1, ACE_TEXT_ALWAYS_CHAR (Common_OS_Tools::GUIDToString (MEDIASUBTYPE_PCM).c_str ()),
                      2, -1,
                      -1);
  gtk_list_store_append (listStore_in, &iterator);
  gtk_list_store_set (listStore_in, &iterator,
                      0, Stream_MediaFramework_Tools::mediaSubTypeToString (MEDIASUBTYPE_IEEE_FLOAT, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str (),
                      1, ACE_TEXT_ALWAYS_CHAR (Common_OS_Tools::GUIDToString (MEDIASUBTYPE_IEEE_FLOAT).c_str ()),
                      2, -1,
                      -1);
#else
  for (int i = 0;
       i <= static_cast<int> (SND_PCM_FORMAT_LAST);
       ++i)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, snd_pcm_format_description (static_cast<enum _snd_pcm_format> (i)),
                        1, snd_pcm_format_name (static_cast<enum _snd_pcm_format> (i)),
                        2, i,
                        -1);
  } // end FOR
#endif // ACE_WIN32 || ACE_WIN64
  return true;
}

bool
load_all_sample_rates (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_all_sample_rates"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  std::vector<unsigned int> rates_a = {4000, 8000, 11025, 22050, 44100, 48000, 96000};
  std::ostringstream converter;
  GtkTreeIter iterator;
  for (std::vector<unsigned int>::const_iterator iterator_2 = rates_a.begin ();
        iterator_2 != rates_a.end ();
        ++iterator_2)
  {
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  } // end FOR
  return true;
}

bool
load_all_sample_resolutions (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_all_sample_resolutions"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  std::vector<unsigned int> resolutions_a = {8, 16};
  //if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IEEE_FLOAT))
  //{
  //  resolutions_a.clear ();
  //  resolutions_a.push_back (32);
  //} // end IF
  std::ostringstream converter;
  GtkTreeIter iterator;
  for (std::vector<unsigned int>::const_iterator iterator_2 = resolutions_a.begin ();
        iterator_2 != resolutions_a.end ();
        ++iterator_2)
  {
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  } // end FOR

  return true;
}

bool
load_all_channels (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_all_channels"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  std::vector<unsigned int> channels_a = {1, 2};
  std::ostringstream converter;
  GtkTreeIter iterator;
  for (std::vector<unsigned int>::const_iterator iterator_2 = channels_a.begin ();
        iterator_2 != channels_a.end ();
        ++iterator_2)
  {
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  } // end FOR

  return true;
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
load_formats (UINT deviceId_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  WAVEINCAPS capabilities_s;
  ACE_OS::memset (&capabilities_s, 0, sizeof (WAVEINCAPS));
  MMRESULT result_2 = waveInGetDevCaps (deviceId_in,
                                        &capabilities_s,
                                        sizeof (WAVEINCAPS));
  if (unlikely (result_2 != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveInGetErrorText (result_2, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waveInGetDevCaps(%d): \"%s\", aborting\n"),
                deviceId_in, ACE_TEXT (error_msg_a)));
    return false;
  } // end IF
  if (unlikely (!capabilities_s.dwFormats))
    return false;

  // *TODO*: check format support
  GtkTreeIter iterator;
  gtk_list_store_append (listStore_in, &iterator);
  gtk_list_store_set (listStore_in, &iterator,
                      0, Stream_MediaFramework_Tools::mediaSubTypeToString (MEDIASUBTYPE_PCM, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str (),
                      1, ACE_TEXT_ALWAYS_CHAR (Common_OS_Tools::GUIDToString (MEDIASUBTYPE_PCM).c_str ()),
                      2, -1,
                      -1);

  return true;
}

bool
load_formats (REFGUID deviceId_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  struct tWAVEFORMATEX* audio_info_p =
    Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat (deviceId_in);
  if (unlikely (!audio_info_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat(%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceId_in).c_str ())));
    return false;
  } // end IF
  struct _GUID GUID_s =
    Stream_MediaFramework_DirectShow_Tools::toSubType (*audio_info_p);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::toSubType(), aborting\n")));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  CoTaskMemFree (audio_info_p); audio_info_p = NULL;

  GtkTreeIter iterator;
  gtk_list_store_append (listStore_in, &iterator);
  gtk_list_store_set (listStore_in, &iterator,
                      0, Stream_MediaFramework_Tools::mediaSubTypeToString (GUID_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str (),
                      1, ACE_TEXT_ALWAYS_CHAR (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                      2, -1,
                      -1);

  return true;
}

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
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
      continue;
    } // end IF

    GUIDs.insert (media_type_p->subtype);
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
  } // end FOR

  GtkTreeIter iterator;
  for (std::set<GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ACE_TEXT_ALWAYS_CHAR (Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                        1, ACE_TEXT_ALWAYS_CHAR (Common_OS_Tools::GUIDToString (*iterator_2).c_str ()),
                        2, -1,
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

  HRESULT result = E_FAIL;
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
  ACE_ASSERT (presentation_descriptor_p);
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
  ACE_ASSERT (stream_descriptor_p);
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
  ACE_ASSERT (media_type_handler_p);
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
    ACE_ASSERT (media_type_p);

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
                        0, ACE_TEXT_ALWAYS_CHAR (Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                        1, ACE_TEXT_ALWAYS_CHAR (Common_OS_Tools::GUIDToString (*iterator_2).c_str ()),
                        2, -1,
                        -1);
  } // end FOR

  return true;
}

bool
load_sample_rates (UINT deviceId_in,
                   REFGUID mediaSubType_in,
                   GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  ACE_UNUSED_ARG (mediaSubType_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  WAVEINCAPS capabilities_s;
  ACE_OS::memset (&capabilities_s, 0, sizeof (WAVEINCAPS));
  MMRESULT result_2 = waveInGetDevCaps (deviceId_in,
                                        &capabilities_s,
                                        sizeof (WAVEINCAPS));
  if (unlikely (result_2 != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveInGetErrorText (result_2, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waveInGetDevCaps(%d): \"%s\", aborting\n"),
                deviceId_in, ACE_TEXT (error_msg_a)));
    return false;
  } // end IF

  unsigned int sample_rate = 0;
  std::ostringstream converter;
  GtkTreeIter iterator;
  if (capabilities_s.dwFormats & 0x0000000F)
  { sample_rate = 11025;
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz\n"),
    //            ACE_TEXT (capabilities_s.szPname), sample_rate));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << sample_rate;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, sample_rate,
                        -1);
  } // end IF
  if (capabilities_s.dwFormats & 0x000000F0)
  { sample_rate = 22050;
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz\n"),
    //            ACE_TEXT (capabilities_s.szPname), sample_rate));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << sample_rate;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, sample_rate,
                        -1);
  } // end IF
  if (capabilities_s.dwFormats & 0x00000F00)
  { sample_rate = 44100;
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz\n"),
    //            ACE_TEXT (capabilities_s.szPname), sample_rate));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << sample_rate;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, sample_rate,
                        -1);
  } // end IF
  if (capabilities_s.dwFormats & 0x0000F000)
  { sample_rate = 48000;
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz\n"),
    //            ACE_TEXT (capabilities_s.szPname), sample_rate));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << sample_rate;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, sample_rate,
                        -1);
  } // end IF
  if (capabilities_s.dwFormats & 0x000F0000)
  { sample_rate = 96000;
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz\n"),
    //            ACE_TEXT (capabilities_s.szPname), sample_rate));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << sample_rate;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, sample_rate,
                        -1);
  } // end IF

  return true;
}

bool
load_sample_rates (REFGUID deviceId_in,
                   REFGUID mediaSubType_in,
                   GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  struct tWAVEFORMATEX* audio_info_p =
    Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat (deviceId_in);
  if (unlikely (!audio_info_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat(%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceId_in).c_str ())));
    return false;
  } // end IF
  struct _GUID GUID_s =
    Stream_MediaFramework_DirectShow_Tools::toSubType (*audio_info_p);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::toSubType(), aborting\n")));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  if (!InlineIsEqualGUID (GUID_s, mediaSubType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown sub type (was: %s; expected: %s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                ACE_TEXT (Common_OS_Tools::GUIDToString (mediaSubType_in).c_str ())));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF

  std::ostringstream converter;
  converter << audio_info_p->nSamplesPerSec;

  GtkTreeIter iterator;
  gtk_list_store_append (listStore_in, &iterator);
  gtk_list_store_set (listStore_in, &iterator,
                      0, converter.str ().c_str (),
                      1, audio_info_p->nSamplesPerSec,
                      -1);

  CoTaskMemFree (audio_info_p); audio_info_p = NULL;

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
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
      continue;
    } // end IF

    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    sample_rates.insert (waveformatex_p->nSamplesPerSec);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
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
                   REFGUID mediaSubType_in,
                   GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  // sanity check(s)
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (IMFMediaSource_in);
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
  ACE_ASSERT (presentation_descriptor_p);
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
  ACE_ASSERT (stream_descriptor_p);
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
  ACE_ASSERT (media_type_handler_p);
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
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
      media_type_p->Release (); media_type_p = NULL;
      media_type_handler_p->Release (); media_type_handler_p = NULL;
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
        media_type_p->Release (); media_type_p = NULL;
        media_type_handler_p->Release (); media_type_handler_p = NULL;
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
load_sample_resolutions (UINT deviceId_in,
                         REFGUID mediaSubType_in,
                         unsigned int sampleRate_in,
                         GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_resolutions"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  WAVEINCAPS capabilities_s;
  ACE_OS::memset (&capabilities_s, 0, sizeof (WAVEINCAPS));
  MMRESULT result_2 = waveInGetDevCaps (deviceId_in,
                                        &capabilities_s,
                                        sizeof (WAVEINCAPS));
  if (unlikely (result_2 != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveInGetErrorText (result_2, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waveInGetDevCaps(%d): \"%s\", aborting\n"),
                deviceId_in, ACE_TEXT (error_msg_a)));
    return false;
  } // end IF

  std::ostringstream converter;
  GtkTreeIter iterator;
  if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IEEE_FLOAT))
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << 32;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, 8,
                        -1);
    return true;
  } // end IF

  switch (sampleRate_in)
  {
    case 11025:
    {
      if ((capabilities_s.dwFormats & WAVE_FORMAT_1M08) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_1S08))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 8));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 8;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 8,
                            -1);
      } // end IF
      if ((capabilities_s.dwFormats & WAVE_FORMAT_1M16) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_1S16))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 16));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 16;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 16,
                            -1);
      } // end IF
      break;
    }
    case 22050:
    {
      if ((capabilities_s.dwFormats & WAVE_FORMAT_2M08) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_2S08))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 8));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 8;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 8,
                            -1);
      } // end IF
      if ((capabilities_s.dwFormats & WAVE_FORMAT_2M16) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_2S16))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 16));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 16;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 16,
                            -1);
      } // end IF
      break;
    }
    case 44100:
    {
      if ((capabilities_s.dwFormats & WAVE_FORMAT_4M08) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_4S08))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 8));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 8;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 8,
                            -1);
      } // end IF
      if ((capabilities_s.dwFormats & WAVE_FORMAT_4M16) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_4S16))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 16));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 16;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 16,
                            -1);
      } // end IF
      break;
    }
    case 48000:
    {
      if ((capabilities_s.dwFormats & WAVE_FORMAT_48M08) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_48S08))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 8));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 8;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 8,
                            -1);
      } // end IF
      if ((capabilities_s.dwFormats & WAVE_FORMAT_48M16) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_48S16))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 16));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 16;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 16,
                            -1);
      } // end IF
      break;
    }
    case 96000:
    {
      if ((capabilities_s.dwFormats & WAVE_FORMAT_96M08) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_96S08))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 8));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 8;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 8,
                            -1);
      } // end IF
      if ((capabilities_s.dwFormats & WAVE_FORMAT_96M16) ||
          (capabilities_s.dwFormats & WAVE_FORMAT_96S16))
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits\n"),
        //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, 16));
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << 16;
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, converter.str ().c_str (),
                            1, 16,
                            -1);
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown sample rate (was: %ukHz), aborting\n"),
                  sampleRate_in));
      return false;
    }
  } // end SWITCH

  return true;
}

bool
load_sample_resolutions (REFGUID deviceId_in,
                         REFGUID mediaSubType_in,
                         unsigned int sampleRate_in,
                         GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_resolutions"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  struct tWAVEFORMATEX* audio_info_p =
    Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat (deviceId_in);
  if (unlikely (!audio_info_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat(%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceId_in).c_str ())));
    return false;
  } // end IF
  struct _GUID GUID_s =
    Stream_MediaFramework_DirectShow_Tools::toSubType (*audio_info_p);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::toSubType(), aborting\n")));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  if (!InlineIsEqualGUID (GUID_s, mediaSubType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown sub type (was: %s; expected: %s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                ACE_TEXT (Common_OS_Tools::GUIDToString (mediaSubType_in).c_str ())));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  if (audio_info_p->nSamplesPerSec != sampleRate_in)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown sample rate (was: %u; expected: %u), aborting\n"),
                audio_info_p->nSamplesPerSec,
                sampleRate_in));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF

  std::ostringstream converter;
  converter << audio_info_p->wBitsPerSample;

  GtkTreeIter iterator;
  gtk_list_store_append (listStore_in, &iterator);
  gtk_list_store_set (listStore_in, &iterator,
                      0, converter.str ().c_str (),
                      1, audio_info_p->wBitsPerSample,
                      -1);

  CoTaskMemFree (audio_info_p); audio_info_p = NULL;

  return true;
}

bool
load_sample_resolutions (IAMStreamConfig* IAMStreamConfig_in,
                         REFGUID mediaSubType_in,
                         unsigned int sampleRate_in,
                         GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_resolutions"));

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
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
      continue;
    } // end IF
    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    if (waveformatex_p->nSamplesPerSec != sampleRate_in)
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
      continue;
    } // end IF
    sample_resolutions.insert (waveformatex_p->wBitsPerSample);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
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
                         REFGUID mediaSubType_in,
                         unsigned int sampleRate_in,
                         GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_resolutions"));

  // sanity check(s)
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (IMFMediaSource_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  Stream_MediaFramework_Sound_SampleResolutions_t sample_resolutions;
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
  ACE_ASSERT (presentation_descriptor_p);
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
  ACE_ASSERT (stream_descriptor_p);
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
  ACE_ASSERT (media_type_handler_p);
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
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
      media_type_p->Release (); media_type_p = NULL;
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                      &sample_rate);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_p->Release (); media_type_p = NULL;
      media_type_handler_p->Release (); media_type_handler_p = NULL;
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
  for (Stream_MediaFramework_Sound_SampleResolutionsIterator_t iterator_2 = sample_resolutions.begin ();
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
load_channels (UINT deviceId_in,
               REFGUID mediaSubType_in,
               unsigned int sampleRate_in,
               unsigned int bitsPerSample_in,
               GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_channels"));

  ACE_UNUSED_ARG (mediaSubType_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  WAVEINCAPS capabilities_s;
  ACE_OS::memset (&capabilities_s, 0, sizeof (WAVEINCAPS));
  MMRESULT result_2 = waveInGetDevCaps (deviceId_in,
                                        &capabilities_s,
                                        sizeof (WAVEINCAPS));
  if (unlikely (result_2 != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveInGetErrorText (result_2, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waveInGetDevCaps(%d): \"%s\", aborting\n"),
                deviceId_in, ACE_TEXT (error_msg_a)));
    return false;
  } // end IF

  std::ostringstream converter;
  GtkTreeIter iterator;
  switch (sampleRate_in)
  {
    case 11025:
    {
      switch (bitsPerSample_in)
      {
        case 8:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_1M08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_1S08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        case 16:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_1M16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_1S16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown bits/sample (was: %u), aborting\n"),
                      bitsPerSample_in));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 22050:
    {
      switch (bitsPerSample_in)
      {
        case 8:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_2M08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_2S08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        case 16:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_2M16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_2S16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown bits/sample (was: %u), aborting\n"),
                      bitsPerSample_in));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 44100:
    {
      switch (bitsPerSample_in)
      {
        case 8:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_4M08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_4S08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        case 16:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_4M16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_4S16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown bits/sample (was: %u), aborting\n"),
                      bitsPerSample_in));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 48000:
    {
      switch (bitsPerSample_in)
      {
        case 8:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_48M08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_48S08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        case 16:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_48M16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_48S16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown bits/sample (was: %u), aborting\n"),
                      bitsPerSample_in));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 96000:
    {
      switch (bitsPerSample_in)
      {
        case 8:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_96M08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_96S08)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        case 16:
        {
          if (capabilities_s.dwFormats & WAVE_FORMAT_96M16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 1));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 1;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 1,
                                -1);
          } // end IF
          if (capabilities_s.dwFormats & WAVE_FORMAT_96S16)
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("found device \"%s\" sample rate: %ukHz, resolution: %u bits, channel(s): %u\n"),
            //            ACE_TEXT (capabilities_s.szPname), sampleRate_in, bitsPerSample_in, 2));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << 2;
            gtk_list_store_append (listStore_in, &iterator);
            gtk_list_store_set (listStore_in, &iterator,
                                0, converter.str ().c_str (),
                                1, 2,
                                -1);
          } // end IF
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown bits/sample (was: %u), aborting\n"),
                      bitsPerSample_in));
          return false;
        }
      } // end SWITCH
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown sample rate (was: %ukHz), aborting\n"),
                  sampleRate_in));
      return false;
    }
  } // end SWITCH

  return true;
}

bool
load_channels (REFGUID deviceId_in,
               REFGUID mediaSubType_in,
               unsigned int sampleRate_in,
               unsigned int bitsPerSample_in,
               GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_channels"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  struct tWAVEFORMATEX* audio_info_p =
    Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat (deviceId_in);
  if (unlikely (!audio_info_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat(%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceId_in).c_str ())));
    return false;
  } // end IF
  struct _GUID GUID_s =
    Stream_MediaFramework_DirectShow_Tools::toSubType (*audio_info_p);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::toSubType(), aborting\n")));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  if (!InlineIsEqualGUID (GUID_s, mediaSubType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown sub type (was: %s; expected: %s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                ACE_TEXT (Common_OS_Tools::GUIDToString (mediaSubType_in).c_str ())));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  if (audio_info_p->nSamplesPerSec != sampleRate_in)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown sample rate (was: %u; expected: %u), aborting\n"),
                audio_info_p->nSamplesPerSec,
                sampleRate_in));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  if (audio_info_p->wBitsPerSample != bitsPerSample_in)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown sample resolution (was: %u; expected: %u), aborting\n"),
                audio_info_p->wBitsPerSample,
                bitsPerSample_in));
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF

  std::ostringstream converter;
  converter << audio_info_p->nChannels;

  GtkTreeIter iterator;
  gtk_list_store_append (listStore_in, &iterator);
  gtk_list_store_set (listStore_in, &iterator,
                      0, converter.str ().c_str (),
                      1, audio_info_p->nChannels,
                      -1);

  CoTaskMemFree (audio_info_p); audio_info_p = NULL;

  return true;
}

bool
load_channels (IAMStreamConfig* IAMStreamConfig_in,
               REFGUID mediaSubType_in,
               unsigned int sampleRate_in,
               unsigned int bitsPerSample_in,
               GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_channels"));

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
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
      continue;
    } // end IF
    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    if (waveformatex_p->nSamplesPerSec != sampleRate_in)
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
      continue;
    } // end IF
    channels.insert (waveformatex_p->nChannels);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p, true);
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
               REFGUID mediaSubType_in,
               unsigned int sampleRate_in,
               unsigned int bitsPerSample_in,
               GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_channels"));

  // sanity check(s)
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (IMFMediaSource_in);
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
  ACE_ASSERT (presentation_descriptor_p);
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
  ACE_ASSERT (stream_descriptor_p);
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
  ACE_ASSERT (media_type_handler_p);
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
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
    if (result != S_OK)
      break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_p->Release (); media_type_p = NULL;
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                      &sample_rate);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_p->Release (); media_type_p = NULL;
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                      &bits_per_sample);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_p->Release (); media_type_p = NULL;
      media_type_handler_p->Release (); media_type_handler_p = NULL;
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
        media_type_p->Release (); media_type_p = NULL;
        media_type_handler_p->Release (); media_type_handler_p = NULL;
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
              enum _snd_pcm_access access_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // sanity check(s)
  ACE_ASSERT (handle_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  enum _snd_pcm_format format_e = SND_PCM_FORMAT_UNKNOWN;
  struct _snd_pcm_hw_params* format_p = NULL;
  snd_pcm_format_mask_t* format_mask_p = NULL;
  std::set<snd_pcm_format_t> formats_supported;

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
                                         access_in);
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
       i <= static_cast<int> (SND_PCM_FORMAT_LAST);
       ++i)
  {
    format_e = static_cast<enum _snd_pcm_format> (i);
    result =
//        snd_pcm_hw_params_test_format (handle_in,
//                                       format_in,
//                                       static_cast<enum _snd_pcm_format> (i));
        snd_pcm_format_mask_test (format_mask_p,
                                  format_e);
    if (result)
      formats_supported.insert (format_e);
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("testing \"%s\": \"%s\"...\n"),
//                ACE_TEXT (snd_pcm_format_name (format_e)),
//                (result ? ACE_TEXT ("supported") : ACE_TEXT ("not supported"))));
  } // end FOR
  snd_pcm_format_mask_free (format_mask_p); format_mask_p = NULL;

  for (std::set<snd_pcm_format_t>::const_iterator iterator_2 = formats_supported.begin ();
       iterator_2 != formats_supported.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, snd_pcm_format_description (*iterator_2),
                        1, snd_pcm_format_name (*iterator_2),
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
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  std::ostringstream converter;
  if (!handle_in)
  {
    std::vector<unsigned int> rates_a = { 4000, 8000, 11025, 22050, 44100, 48000, 96000 };
    for (std::vector<unsigned int>::const_iterator iterator_2 = rates_a.begin ();
          iterator_2 != rates_a.end ();
          ++iterator_2)
    {
      converter << *iterator_2;
      gtk_list_store_append (listStore_in, &iterator);
      gtk_list_store_set (listStore_in, &iterator,
                          0, converter.str ().c_str (),
                          1, *iterator_2,
                          -1);
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    } // end FOR
    return true;
  } // end IF

  struct _snd_pcm_hw_params* format_p = NULL;
  unsigned int rate_min, rate_max;
  int subunit_direction = 0;
  std::set<unsigned int> sample_rates_supported;

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

//   result = snd_pcm_hw_params_set_access (handle_in,
//                                          format_p,
//                                          mediaType_in.access);
//   if (result < 0)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
//                 ACE_TEXT (snd_strerror (result))));
//     goto error;
//   } // end IF
  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         mediaType_in.format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_rate_resample (handle_in,
                                                format_p,
                                                0);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_rate_resample(0): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
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
    std::vector<unsigned int> rates_norm_a = { 4000, 8000, 11025, 22050, 44100, 48000, 96000, 192000 };
    for (std::vector<unsigned int>::const_iterator iterator = rates_norm_a.begin ();
         iterator != rates_norm_a.end ();
         ++iterator)
    {
      result = snd_pcm_hw_params_test_rate (handle_in,
                                            format_p,
                                            *iterator,
                                            0);
      if (result == 0)
        sample_rates_supported.insert (*iterator);
    } // end FOR
  } // end IF
  else
    sample_rates_supported.insert (rate_min);
  snd_pcm_hw_params_free (format_p); format_p = NULL;

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
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  std::ostringstream converter;
  if (!handle_in)
  {
    std::vector<unsigned int> resolutions_a = {8, 16, 32};
    for (std::vector<unsigned int>::const_iterator iterator_2 = resolutions_a.begin ();
          iterator_2 != resolutions_a.end ();
          ++iterator_2)
    {
      converter << *iterator_2;
      gtk_list_store_append (listStore_in, &iterator);
      gtk_list_store_set (listStore_in, &iterator,
                          0, converter.str ().c_str (),
                          1, *iterator_2,
                          -1);
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    } // end FOR
    return true;
  } // end IF

  int subunit_direction = 0;
  struct _snd_pcm_hw_params* format_p = NULL;
  std::set<int> resolutions_supported;

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

  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         mediaType_in.format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_rate (handle_in,
                                  format_p,
                                  const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).rate,
                                  subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_rate(): \"%s\", aborting\n"),
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
  snd_pcm_hw_params_free (format_p); format_p = NULL;

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
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  std::ostringstream converter;
  if (!handle_in)
  {
    std::vector<unsigned int> channels_a = {1, 2};
    for (std::vector<unsigned int>::const_iterator iterator_2 = channels_a.begin ();
         iterator_2 != channels_a.end ();
         ++iterator_2)
    {
      converter << *iterator_2;
      gtk_list_store_append (listStore_in, &iterator);
      gtk_list_store_set (listStore_in, &iterator,
                          0, converter.str ().c_str (),
                          1, *iterator_2,
                          -1);
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    } // end FOR
    return true;
  } // end IF

  struct _snd_pcm_hw_params* format_p = NULL;
  int subunit_direction = 0;
  unsigned int channels_min, channels_max;
  std::set<unsigned int> channels_supported;

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

//   result = snd_pcm_hw_params_set_access (handle_in,
//                                          format_p,
//                                          mediaType_in.access);
//   if (result < 0)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
//                 ACE_TEXT (snd_strerror (result))));
//     goto error;
//   } // end IF
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
      snd_pcm_hw_params_set_rate (handle_in,
                                  format_p,
                                  const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).rate,
                                  subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_rate(): \"%s\", aborting\n"),
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
    std::vector<unsigned int> channels_norm_a = { 1, 2, 3, 4, 5, 6, 7, 8 };
    for (std::vector<unsigned int>::const_iterator iterator = channels_norm_a.begin ();
         iterator != channels_norm_a.end ();
         ++iterator)
    {
      result = snd_pcm_hw_params_test_channels (handle_in,
                                                format_p,
                                                *iterator);
      if (result == 0)
        channels_supported.insert (*iterator);
    } // end FOR
  } // end IF
  else
    channels_supported.insert (channels_min);
  snd_pcm_hw_params_free (format_p); format_p = NULL;

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
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::load_audio_effects"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  std::string friendly_name_string;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      IEnumDMO* enum_DMO_p = NULL;
      int result_3 = -1;
      CLSID class_id = GUID_NULL;
      WCHAR* string_p = NULL;

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

      while (S_OK == enum_DMO_p->Next (1,
                                       &class_id,
                                       &string_p,
                                       NULL))
      { ACE_ASSERT (string_p);
        friendly_name_string =
           ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (string_p));
        CoTaskMemFree (string_p); string_p = NULL;

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, friendly_name_string.c_str (),
                            1, Common_OS_Tools::GUIDToString (class_id).c_str (),
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
      MFT_REGISTER_TYPE_INFO mft_register_type_info =
        { MFMediaType_Audio, MFAudioFormat_PCM };
      UINT32 flags = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      flags = (MFT_ENUM_FLAG_SYNCMFT        |
               MFT_ENUM_FLAG_ASYNCMFT       |
               MFT_ENUM_FLAG_HARDWARE       |
               MFT_ENUM_FLAG_FIELDOFUSE     |
               MFT_ENUM_FLAG_LOCALMFT       |
               MFT_ENUM_FLAG_TRANSCODE_ONLY |
               MFT_ENUM_FLAG_SORTANDFILTER);
      IMFActivate** activate_a = NULL;
#else
      CLSID* activate_a = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      struct _GUID GUID_s = GUID_NULL;

      if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_AUDIO_EFFECT,
                                                              flags,
                                                              &mft_register_type_info,    // input type
                                                              NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                              activate_a,                 // array of effects
#else
                                                              NULL,                       // attributes
                                                              activate_a,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                              item_count))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                    ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_AUDIO_EFFECT).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (MFAudioFormat_PCM, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (activate_a);

      for (UINT32 i = 0; i < item_count; i++)
      {
        friendly_name_string =
          Stream_MediaFramework_MediaFoundation_Tools::toString (activate_a[i]);
        result_2 = activate_a[i]->GetGUID (MFT_TRANSFORM_CLSID_Attribute,
                                           &GUID_s);
        ACE_ASSERT (SUCCEEDED (result_2));
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, friendly_name_string.c_str (),
                            1, Common_OS_Tools::GUIDToString (GUID_s).c_str (),
                            -1);
      } // end FOR

      result = true;

error:
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      for (UINT32 i = 0; i < item_count; i++)
        activate_a[i]->Release ();
      CoTaskMemFree (activate_a); activate_a = NULL;
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
                                      command_output_string,
                                      true)) // return stdout
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::command(\"%s\"), aborting\n"),
                ACE_TEXT (command_line_string.c_str ())));
    result = false;
    goto continue_;
  } // end IF
  start_position =
      command_output_string.find (ACE_TEXT_ALWAYS_CHAR ("EFFECTS: "));
  if (start_position == std::string::npos)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to parse shell command output (was: \"%s\"), aborting\n"),
                ACE_TEXT (command_output_string.c_str ())));
    result = false;
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
    result = false;
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
    result = false;
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
#endif // ACE_WIN32 || ACE_WIN64
continue_:

  return result;
}

unsigned int
get_buffer_size (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::get_buffer_size"));

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
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
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
  struct _GUID GUID_s = Common_OS_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), aborting\n"),
                ACE_TEXT (format_string.c_str ())));
    return 0;
  } // end IF
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

  unsigned int bps = (sample_rate * (bits_per_sample / 8) * channels);
  // *IMPORTANT NOTE*: lower buffer sizes result in lower latency
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: the modifier is needed to prevent crackle on Win32... :-(
  return static_cast<unsigned int> ((STREAM_DEC_NOISE_BUFFER_LATENCY_MS * bps * 2.0f) / (float)MILLISECONDS);
#else
  ACE_UNUSED_ARG (format_e);
  return static_cast<unsigned int> ((STREAM_DEC_NOISE_BUFFER_LATENCY_MS * bps) / (float)1000);
#endif // ACE_WIN32 || ACE_WIN64
}

void
update_media_type (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::update_media_type"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.cbFormat == sizeof (struct tWAVEFORMATEX));
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      // step1: set missing format properties
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      waveformatex_p->nBlockAlign =
        (waveformatex_p->nChannels * (waveformatex_p->wBitsPerSample / 8));
      waveformatex_p->nAvgBytesPerSec =
        (waveformatex_p->nSamplesPerSec * waveformatex_p->nBlockAlign);

      // step2: initialize output format
      Stream_MediaFramework_DirectShow_Tools::free ((*directshow_modulehandler_configuration_iterator).second.second->outputFormat);
      Stream_MediaFramework_DirectShow_Tools::copy (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                                                    (*directshow_modulehandler_configuration_iterator).second.second->outputFormat);

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      // step1: set missing format properties
      UINT32 number_of_channels, bits_per_sample, sample_rate;
      HRESULT result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                                                                            &sample_rate);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                                            &bits_per_sample);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                            &number_of_channels);
      ACE_ASSERT (SUCCEEDED (result));
      unsigned int block_alignment_i = number_of_channels * (bits_per_sample / 8);
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_BLOCK_ALIGNMENT,
                                                                                                            block_alignment_i);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_SAMPLE_SIZE,
                                                                                                            block_alignment_i);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_VALID_BITS_PER_SAMPLE,
                                                                                                            bits_per_sample);
      ACE_ASSERT (SUCCEEDED (result));
      //UINT32 channel_mask_i = (SPEAKER_FRONT_LEFT |
      //                         SPEAKER_FRONT_RIGHT);
      //result =
      //  mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_CHANNEL_MASK,
      //                                                                                                      channel_mask_i);
      //ACE_ASSERT (SUCCEEDED (result));
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                                                                                            sample_rate * block_alignment_i);
      ACE_ASSERT (SUCCEEDED (result));

      // step2: initialize output format
      if ((*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat)
      {
        (*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat->Release (); (*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat = NULL;
      } // end IF
      (*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat =
        Stream_MediaFramework_MediaFoundation_Tools::copy (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat);

      if (mediafoundation_ui_cb_data_p->configuration->mediaFoundationConfiguration.mediaType)
      {
        mediafoundation_ui_cb_data_p->configuration->mediaFoundationConfiguration.mediaType->Release (); mediafoundation_ui_cb_data_p->configuration->mediaFoundationConfiguration.mediaType = NULL;
      } // end IF
      mediafoundation_ui_cb_data_p->configuration->mediaFoundationConfiguration.mediaType =
        Stream_MediaFramework_MediaFoundation_Tools::copy (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->mediaFoundationConfiguration.mediaType);

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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  // step2: initialize output format
  (*modulehandler_configuration_iterator).second.second->outputFormat =
    ui_cb_data_p->configuration->streamConfiguration.configuration_->format;
#endif // ACE_WIN32 || ACE_WIN64
}

void
update_buffer_size (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::update_buffer_size"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
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

  // initialize return value(s)
  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
  bool COM_initialized = Common_Tools::initializeCOM ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  struct Test_U_UI_ThreadData* thread_data_base_p =
      static_cast<struct Test_U_UI_ThreadData*> (arg_in);
  ACE_ASSERT (thread_data_base_p);

  Common_UI_GTK_BuildersConstIterator_t iterator;
  Common_UI_GTK_State_t* state_p = NULL;
  std::ostringstream converter;
  Test_U_AudioEffect_SessionData* session_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  const Test_U_AudioEffect_DirectShow_SessionData_t* directshow_session_data_container_p =
    NULL;
  Test_U_AudioEffect_DirectShow_SessionData* directshow_session_data_p =
    NULL;
  const Test_U_AudioEffect_MediaFoundation_SessionData_t* mediafoundation_session_data_container_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_SessionData* mediafoundation_session_data_p =
    NULL;
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      struct Test_U_AudioEffect_DirectShow_ThreadData* thread_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_ThreadData*> (arg_in);
      directshow_ui_cb_data_p = thread_data_p->CBData;
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      state_p = directshow_ui_cb_data_p->UIState;
      ACE_ASSERT (state_p);
      iterator =
        state_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      ACE_ASSERT (iterator != state_p->builders.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      struct Test_U_AudioEffect_MediaFoundation_ThreadData* thread_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_ThreadData*> (arg_in);
      mediafoundation_ui_cb_data_p = thread_data_p->CBData;
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      state_p = mediafoundation_ui_cb_data_p->UIState;
      ACE_ASSERT (state_p);
      iterator =
        state_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      ACE_ASSERT (iterator != state_p->builders.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  thread_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p = NULL;
  const Test_U_AudioEffect_SessionData_t* session_data_container_p = NULL;

  // sanity check(s)
  struct Test_U_AudioEffect_ThreadData* thread_data_p =
    static_cast<struct Test_U_AudioEffect_ThreadData*> (arg_in);
  ui_cb_data_p = thread_data_p->CBData;
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  state_p = ui_cb_data_p->UIState;
  ACE_ASSERT (state_p);
  iterator =
    state_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_p->builders.end ());
#endif // ACE_WIN32 || ACE_WIN64

//  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;

  // retrieve progress bar handle
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
//    progress_bar_p =
//      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
//                                                ACE_TEXT_ALWAYS_CHAR (TEST_USTREAM_UI_GTK_PROGRESSBAR_NAME)));
//    ACE_ASSERT (progress_bar_p);

  // generate context id
  statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  bool result_2 = false;
  Stream_IStream_t* istream_p = NULL;
  Stream_IStreamControlBase* istream_control_p = NULL;
  const Stream_Module_t* module_p = NULL;
  Test_U_Common_ISet_t* resize_notification_p = NULL;
  Common_IDispatch* dispatch_p = NULL;
  guint event_source_id = 0;
  struct Test_U_AudioEffect_UI_CBDataBase* ui_data_base_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Test_U_AudioEffect_DirectShow_Stream::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_U_AudioEffect_DirectShow_Stream::IINITIALIZE_T*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (iinitialize_p);
      result_2 =
        iinitialize_p->initialize (directshow_ui_cb_data_p->configuration->streamConfiguration);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      istream_control_p = directshow_ui_cb_data_p->stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Test_U_AudioEffect_MediaFoundation_Stream::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_U_AudioEffect_MediaFoundation_Stream::IINITIALIZE_T*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (iinitialize_p);
      result_2 =
        iinitialize_p->initialize (mediafoundation_ui_cb_data_p->configuration->streamConfiguration);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
      istream_control_p = mediafoundation_ui_cb_data_p->stream;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  thread_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  Test_U_AudioEffect_ALSA_Stream::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Test_U_AudioEffect_ALSA_Stream::IINITIALIZE_T*> (ui_cb_data_p->stream);
  ACE_ASSERT (iinitialize_p);
  result_2 =
    iinitialize_p->initialize (ui_cb_data_p->configuration->streamConfiguration);
  istream_p = dynamic_cast<Stream_IStream_t*> (ui_cb_data_p->stream);
  istream_control_p = ui_cb_data_p->stream;
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
  ACE_ASSERT (resize_notification_p);
  dispatch_p =
    dynamic_cast<Common_IDispatch*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (resize_notification_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p->resizeNotification = resize_notification_p;
      directshow_ui_cb_data_p->spectrumAnalyzerCBData.dispatch = dispatch_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p->resizeNotification = resize_notification_p;
      mediafoundation_ui_cb_data_p->spectrumAnalyzerCBData.dispatch =
        dispatch_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  thread_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  ui_cb_data_p->resizeNotification = resize_notification_p;
  ui_cb_data_p->spectrumAnalyzerCBData.dispatch = dispatch_p;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Common_IGetR_2_T<Test_U_AudioEffect_DirectShow_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_2_T<Test_U_AudioEffect_DirectShow_SessionData_t>*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (iget_p);
      directshow_session_data_container_p = &iget_p->getR_2 ();
      directshow_session_data_p =
        &const_cast<Test_U_AudioEffect_DirectShow_SessionData&> (directshow_session_data_container_p->getR ());
      session_data_p = directshow_session_data_p;
      directshow_ui_cb_data_p->progressData.sessionId = session_data_p->sessionId;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Common_IGetR_2_T<Test_U_AudioEffect_MediaFoundation_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_2_T<Test_U_AudioEffect_MediaFoundation_SessionData_t>*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (iget_p);
      mediafoundation_session_data_container_p = &iget_p->getR_2 ();
      mediafoundation_session_data_p =
        &const_cast<Test_U_AudioEffect_MediaFoundation_SessionData&> (mediafoundation_session_data_container_p->getR ());
      session_data_p = mediafoundation_session_data_p;
      mediafoundation_ui_cb_data_p->progressData.sessionId = session_data_p->sessionId;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  thread_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  iget_p =
    dynamic_cast<Common_IGetR_2_T<Test_U_AudioEffect_SessionData_t>*> (ui_cb_data_p->stream);
  ACE_ASSERT (iget_p);
  session_data_container_p = &iget_p->getR_2 ();
  session_data_p =
      &const_cast<Test_U_AudioEffect_SessionData&> (session_data_container_p->getR ());
  ui_cb_data_p->progressData.sessionId = session_data_p->sessionId;
#endif // ACE_WIN32 || ACE_WIN64
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << session_data_p->sessionId;

  // generate context id
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
  state_p->contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                              gtk_statusbar_get_context_id (statusbar_p,
                                                                            converter.str ().c_str ())));
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  istream_control_p->start ();
  //if (!istream_control_p->isRunning ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Test_U_AudioEffect_Stream::start(): \"%m\", aborting\n")));
  //  goto done;
  //} // end IF
  istream_control_p->wait (true,   // wait for any worker thread(s) ?
                           false,  // wait for upstream (if any) ?
                           false); // wait for downstream (if any) ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p->resizeNotification = NULL;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p->resizeNotification = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  thread_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  ui_cb_data_p->resizeNotification = NULL;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  goto continue_;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      ui_data_base_p = directshow_ui_cb_data_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ui_data_base_p = mediafoundation_ui_cb_data_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  thread_data_base_p->mediaFramework));
      goto continue_;
    }
  } // end SWITCH
#else
  ui_data_base_p = ui_cb_data_p;
#endif // ACE_WIN32 || ACE_WIN64
  // *NOTE*: do not use g_idle_add, because that will never be called;
  //         the system is never idle while in-session, because it's busy
  //         updating the display...
  event_source_id = g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_CAIRO_MS,
                                   idle_session_end_cb,
                                   ui_data_base_p);
  if (event_source_id == 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_timeout_add(idle_session_end_cb): \"%m\", continuing\n")));
  else
  { ACE_ASSERT (state_p);
    state_p->eventSourceIds.insert (event_source_id);
  } // end ELSE

continue_:
  { // synch access
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_p->lock, std::numeric_limits<ACE_THR_FUNC_RETURN>::max ());
    switch (thread_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        directshow_ui_cb_data_p->progressData.completedActions.insert (thread_data_base_p->eventSourceId);
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        mediafoundation_ui_cb_data_p->progressData.completedActions.insert (thread_data_base_p->eventSourceId);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), continuing\n"),
                    thread_data_base_p->mediaFramework));
        break;
      }
    } // end SWITCH
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_p->lock, arg_in);
    ui_cb_data_p->progressData.completedActions.insert (thread_data_base_p->eventSourceId);
#endif // ACE_WIN32 || ACE_WIN64
  } // end lock scope

  // clean up
  delete thread_data_base_p; thread_data_base_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (COM_initialized)
    Common_Tools::finalizeCOM ();
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

//////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  gint sort_column_id; // device
  GtkSortType sort_order;
  enum Stream_MediaFramework_SoundGeneratorType noise_type_e =
    STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_INVALID;
  GtkScale* scale_p = NULL;
  double default_noise_frequency_d = 0.0;
  GtkFileFilter* file_filter_p = NULL;
  std::string filename_string;
  GtkFileChooserButton* file_chooser_button_p = NULL;
  GtkFileChooserDialog* file_chooser_dialog_p = NULL;
  gboolean result = FALSE;
  GtkSizeGroup* size_group_p = NULL;
  GtkButton* button_p = NULL;
  GtkCheckButton* check_button_p = NULL;
  GtkToggleButton* toggle_button_p = NULL;
  long min_level_i = 0, max_level_i = 0, current_level_i = 0;
  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode_2d =
    STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID;
  enum Stream_Visualization_SpectrumAnalyzer_3DMode mode_3d =
    STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID;
  GtkDrawingArea* drawing_area_p = NULL;
  gint tooltip_timeout = COMMON_UI_GTK_TIMEOUT_DEFAULT_WIDGET_TOOLTIP_DELAY_MS;
#if defined (GTKGL_SUPPORT)
  GtkBox* box_p = NULL;
  Common_UI_GTK_GLContextsIterator_t opengl_contexts_iterator;
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea* gl_area_p = NULL;
  GdkGLContext* gl_context_p = NULL;
  GError* error_p = NULL;
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  /* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GDK_GL_NONE. See glXChooseVisual manpage for further
     explanation.
  */
  int gl_attributes_a[] = {GDK_GL_USE_GL,
                           // GDK_GL_BUFFER_SIZE
                           // GDK_GL_LEVEL
                           GDK_GL_RGBA, GDK_GL_DOUBLEBUFFER,
                           //    GDK_GL_STEREO
                           //    GDK_GL_AUX_BUFFERS
                           GDK_GL_RED_SIZE, 1, GDK_GL_GREEN_SIZE, 1,
                           GDK_GL_BLUE_SIZE, 1, GDK_GL_ALPHA_SIZE, 1,
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
                           GDK_GL_NONE};
  GtkGLArea* gl_area_p = NULL;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT
  GdkWindow* window_p = NULL;
  gint n_rows = 0;
  gint index_i = 0;
  bool is_active_b = false;
  GtkRadioButton* radio_button_p = NULL;
  GtkDrawingArea* drawing_area_2 = NULL;
  gulong result_2 = 0;
  bool desensitize_device_combobox_b = false;
  std::string device_identifier_string;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_2; // file writer
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_2; // file writer
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_3; // renderer
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_3; // renderer
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      directshow_modulehandler_configuration_iterator_2 =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator_2 != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      std::string renderer_modulename_string;
      switch (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer)
      {
        case STREAM_DEVICE_RENDERER_WAVEOUT:
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING);
          break;
        case STREAM_DEVICE_RENDERER_WASAPI:
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING);
          break;
        case STREAM_DEVICE_RENDERER_DIRECTSHOW:
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING);
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown renderer type (was: %d), aborting\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer));
          goto error;
        }
      } // end SWITCH
      directshow_modulehandler_configuration_iterator_3 =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (renderer_modulename_string);
      ACE_ASSERT (directshow_modulehandler_configuration_iterator_3 != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      mediafoundation_modulehandler_configuration_iterator_2 =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator_2 != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      mediafoundation_modulehandler_configuration_iterator_3 =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator_3 != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator_2 = // renderer
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator_2 != data_p->configuration->streamConfiguration.end ());
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator_3 = // file writer
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator_3 != data_p->configuration->streamConfiguration.end ());

  desensitize_device_combobox_b = !data_p->switchCaptureDevice;
  device_identifier_string =
    (*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier;
#endif // ACE_WIN32 || ACE_WIN64

  // step1: initialize widgets
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDSAMPLES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDSAMPLES_NAME)));
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
  enum Stream_Device_Capturer capturer_e = STREAM_DEVICE_CAPTURER_INVALID;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      buffer_size =
        directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize;
      capturer_e =
        directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      buffer_size =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize;
      capturer_e =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
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
  GtkTreeIter iterator_2;
  gtk_list_store_append (list_store_p, &iterator_2);
  gtk_list_store_set (list_store_p, &iterator_2,
                      0, ACE_TEXT_ALWAYS_CHAR ("Capture Device"),
                      1, AUDIOEFFECT_SOURCE_DEVICE,
                      -1);
  gtk_list_store_append (list_store_p, &iterator_2);
  gtk_list_store_set (list_store_p, &iterator_2,
                      0, ACE_TEXT_ALWAYS_CHAR ("Noise"),
                      1, AUDIOEFFECT_SOURCE_NOISE,
                      -1);
  gtk_list_store_append (list_store_p, &iterator_2);
  gtk_list_store_set (list_store_p, &iterator_2,
                      0, ACE_TEXT_ALWAYS_CHAR ("File"),
                      1, AUDIOEFFECT_SOURCE_FILE,
                      -1);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  "text", 0,
                                  NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_DEVICE_NAME)));
  ACE_ASSERT (list_store_p);
  sort_column_id = 1; // device
  sort_order = GTK_SORT_DESCENDING;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (capturer_e == STREAM_DEVICE_CAPTURER_WAVEIN)
  {
    sort_column_id = 2; // card
    sort_order = GTK_SORT_ASCENDING;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        sort_column_id, sort_order);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_capture_devices (list_store_p,
                             capturer_e))
#else
  if (!load_capture_devices (list_store_p,
                             device_identifier_string))
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (!device_identifier_string.empty ())
    {
    //  int mode = STREAM_LIB_ALSA_CAPTURE_DEFAULT_MODE;
      int mode = SND_PCM_NONBLOCK         |
                 SND_PCM_NO_AUTO_RESAMPLE |
                 SND_PCM_NO_AUTO_CHANNELS |
                 SND_PCM_NO_SOFTVOL;
    //   if ((*modulehandler_configuration_iterator).second.second->ALSAConfiguration->asynch)
    //     mode |= SND_PCM_ASYNC;
      result =
        snd_pcm_open (&data_p->handle,
                      device_identifier_string.c_str (),
                      SND_PCM_STREAM_CAPTURE,
                      mode);
      if ((result < 0) || !data_p->handle)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_pcm_open(\"%s\",%d) for capture: \"%s\", aborting\n"),
                    ACE_TEXT (device_identifier_string.c_str ()),
                    mode,
                    ACE_TEXT (snd_strerror (result))));
        goto error;
      } // end IF
      (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->handle =
        data_p->handle;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("opened ALSA device (capture) \"%s\"\n"),
                  ACE_TEXT (device_identifier_string.c_str ())));

      goto continue_;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_capture_devices(), aborting\n")));
    goto error;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
continue_:
#endif // ACE_WIN32 || ACE_WIN64

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_DEVICE_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);

  if (desensitize_device_combobox_b)
      gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);

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
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
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
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
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
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
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
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);

  //  GtkHScale* scale_p =
  //    GTK_HSCALE (gtk_builder_get_object ((*iterator).second.second,
  //                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_HSCALE_BOOST_NAME)));
  //  ACE_ASSERT (scale_p);
  //  gtk_range_set_update_policy (GTK_RANGE (scale_p),
  //                               GTK_UPDATE_DELAYED);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration);
      noise_type_e =
        (*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration->type;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration);
      noise_type_e =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration->type;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->generatorConfiguration);
  noise_type_e =
    (*modulehandler_configuration_iterator).second.second->generatorConfiguration->type;
#endif // ACE_WIN32 || ACE_WIN64
  if (noise_type_e != STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_INVALID)
  {
    GtkRadioButton* radio_button_p = NULL;
    switch (noise_type_e)
    {
      case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SAWTOOTH:
        radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SAWTOOTH_NAME)));
        break;
      case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE:
        radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SINUS_NAME)));
        break;
      case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SQUARE:
        radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SQUARE_NAME)));
        break;
      case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_TRIANGLE:
        radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_TRIANGLE_NAME)));
        break;
      case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_NOISE:
        radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_NOISE_NAME)));
        break;
#if defined (LIBNOISE_SUPPORT)
      case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PERLIN_NOISE:
        radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_PERLIN_NAME)));
        break;
#endif // LIBNOISE_SUPPORT
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown noise type (was: %d), aborting\n"),
                    noise_type_e));
        goto error;
      }
    } // end SWITCH
    ACE_ASSERT (radio_button_p);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p), TRUE);
  } // end IF

  scale_p = 
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_HSCALE_SINUS_FREQUENCY_NAME)));
  ACE_ASSERT (scale_p);
  gtk_scale_add_mark (scale_p,
                      //gtk_range_get_value (GTK_RANGE (hscale_p)),
                      TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_FREQUENCY_D,
                      GTK_POS_TOP,
                      NULL);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration);
      default_noise_frequency_d =
        (*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration->frequency;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration);
      default_noise_frequency_d =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration->frequency;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  default_noise_frequency_d =
    (*modulehandler_configuration_iterator).second.second->generatorConfiguration->frequency;
#endif // ACE_WIN32 || ACE_WIN64
  gtk_range_set_value (GTK_RANGE (scale_p),
                       default_noise_frequency_d);

  file_filter_p =
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
  file_filter_p =
    GTK_FILE_FILTER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILEFILTER_MP3_NAME)));
  ACE_ASSERT (file_filter_p);
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("application/octet-stream"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/mp3"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/mpeg"));
  gtk_file_filter_add_pattern (file_filter_p,
                               ACE_TEXT ("*.mp3"));
  gtk_file_filter_set_name (file_filter_p,
                            ACE_TEXT ("MP3 files"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      filename_string =
        (*directshow_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      filename_string =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  filename_string =
    (*modulehandler_configuration_iterator).second.second->fileIdentifier.identifier;
#endif // ACE_WIN32 || ACE_WIN64
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_FILE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser_button_p),
                               file_filter_p);
  file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_FILE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  //gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser_dialog_p),
  //                             file_filter_p);
  if (!filename_string.empty ())
  {
    GFile* file_p = g_file_new_for_path (filename_string.c_str ());
    ACE_ASSERT (file_p);
    GFile* file_2 = g_file_get_parent (file_p);
    ACE_ASSERT (file_2);
    GError* error_p = NULL;
    result =
      gtk_file_chooser_set_file (GTK_FILE_CHOOSER (file_chooser_button_p),
                                 file_p,
                                 &error_p);
    ACE_ASSERT (result && !error_p);
    result =
      gtk_file_chooser_set_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
                                 file_p,
                                 &error_p);
    ACE_ASSERT (result && !error_p);
    g_object_unref (file_p); file_p = NULL;
    g_object_unref (file_2); file_2 = NULL;
  } // end IF

  size_group_p =
    GTK_SIZE_GROUP (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SIZEGROUP_OPTIONS_NAME)));
  ACE_ASSERT (size_group_p);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_DEVICE_SETTINGS_NAME)));
  ACE_ASSERT (button_p);
  gtk_size_group_add_widget (size_group_p, GTK_WIDGET (button_p));
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_EFFECT_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_size_group_add_widget (size_group_p, GTK_WIDGET (check_button_p));
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_MUTE_NAME)));
  ACE_ASSERT (toggle_button_p);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_SAVE_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_size_group_add_widget (size_group_p, GTK_WIDGET (check_button_p));
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_size_group_add_widget (size_group_p, GTK_WIDGET (check_button_p));

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
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
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
    goto error;
  } // end IF

  // get/set render volume level
  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_HSCALE_VOLUME_NAME)));
  ACE_ASSERT (scale_p);
  // *TODO*: select output device
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
  struct _GUID GUID_2 = CLSID_ACEStream_MediaFramework_WASAPI_AudioSession;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { 
      switch (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer)
      {
        case STREAM_DEVICE_RENDERER_WAVEOUT:
        { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
          GUID_s =
            Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID ((*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifier._id,
                                                                                    false); // playback
          GUID_2 = GUID_NULL; // *NOTE*: waveOut devices join the default audio session --> GUID_NULL
          break;
        }
        case STREAM_DEVICE_RENDERER_WASAPI:
        { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
          GUID_s =
            (*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifier._guid;
          break;
        }
        case STREAM_DEVICE_RENDERER_DIRECTSHOW:
        { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
          GUID_s =
            Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID ((*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifier._id,
                                                                                    false); // playback
          GUID_2 = GUID_NULL; // *NOTE*: waveOut devices join the default audio session --> GUID_NULL
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown renderer type (was: %d), aborting\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer));
          goto error;
        }
      } // end SWITCH
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      GUID_s =
        (*mediafoundation_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifier._guid;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
  ISimpleAudioVolume* i_simple_audio_volume_p =
    Stream_MediaFramework_DirectSound_Tools::getSessionVolumeControl (GUID_s,
                                                                      GUID_2);
  if (!i_simple_audio_volume_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getSessionVolumeControl(\"%s\",\"%s\"), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_2).c_str ())));
    goto error;
  } // end IF
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p->renderVolumeControl = i_simple_audio_volume_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p->renderVolumeControl =
        i_simple_audio_volume_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
  float volume_level_f = 0.0;
  HRESULT result_3 =
    i_simple_audio_volume_p->GetMasterVolume (&volume_level_f);
  ACE_ASSERT (SUCCEEDED (result_3));
  gtk_range_set_value (GTK_RANGE (scale_p),
                       static_cast<gdouble> (volume_level_f) * 100.0);
#else
  if (!Stream_MediaFramework_ALSA_Tools::getVolumeLevels ((*modulehandler_configuration_iterator_2).second.second->deviceIdentifier.identifier,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_PLAYBACK_DEFAULT_SELEM_VOLUME_NAME),
                                                          false, // playback
                                                          min_level_i,
                                                          max_level_i,
                                                          current_level_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getVolumeLevels(\"%s\",\"%s\"), continuing\n"),
                ACE_TEXT ((*modulehandler_configuration_iterator_2).second.second->deviceIdentifier.identifier.c_str ()),
                ACE_TEXT (STREAM_LIB_ALSA_PLAYBACK_DEFAULT_SELEM_VOLUME_NAME)));
    goto continue_2;
  } // end IF
  gtk_range_set_range (GTK_RANGE (scale_p),
                       static_cast<gdouble> (min_level_i),
                       static_cast<gdouble> (max_level_i));
  gtk_range_set_increments (GTK_RANGE (scale_p),
                            static_cast<gdouble> (1),
                            static_cast<gdouble> (1));
  gtk_range_set_value (GTK_RANGE (scale_p),
                       static_cast<gdouble> (current_level_i));
continue_2:
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      filename_string =
        ((*directshow_modulehandler_configuration_iterator_2).second.second->fileIdentifier.empty () ? Common_File_Tools::getTempDirectory ()
                                                                                                     : (*directshow_modulehandler_configuration_iterator_2).second.second->fileIdentifier.identifier);
      if (Common_File_Tools::isDirectory (filename_string))
        (*directshow_modulehandler_configuration_iterator_2).second.second->fileIdentifier.clear ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      filename_string =
        ((*mediafoundation_modulehandler_configuration_iterator_2).second.second->fileIdentifier.empty () ? Common_File_Tools::getTempDirectory ()
                                                                                                          : (*mediafoundation_modulehandler_configuration_iterator_2).second.second->fileIdentifier.identifier);
      if (Common_File_Tools::isDirectory (filename_string))
        (*mediafoundation_modulehandler_configuration_iterator_2).second.second->fileIdentifier.clear ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  filename_string =
    ((*modulehandler_configuration_iterator_3).second.second->fileIdentifier.empty () ? Common_File_Tools::getTempDirectory ()
                                                                                      : (*modulehandler_configuration_iterator_3).second.second->fileIdentifier.identifier);
  if (Common_File_Tools::isDirectory (filename_string))
    (*modulehandler_configuration_iterator_3).second.second->fileIdentifier.clear ();
#endif // ACE_WIN32 || ACE_WIN64
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  if (!filename_string.empty ())
  {
    if (!Common_File_Tools::isDirectory (filename_string))
    {
      // *NOTE*: gtk does not complain if the file doesn't exist, but the button
      //         will display "(None)" --> create empty file
      if (!Common_File_Tools::isReadable (filename_string))
        if (!Common_File_Tools::create (filename_string))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_File_Tools::create(\"%s\"): \"%m\", aborting\n"),
                      ACE_TEXT (filename_string.c_str ())));
          goto error;
        } // end IF
      GFile* file_p = g_file_new_for_path (filename_string.c_str ());
      ACE_ASSERT (file_p);
      GFile* file_2 = g_file_get_parent (file_p);
      ACE_ASSERT (file_2);
      GError* error_p = NULL;
      result =
        gtk_file_chooser_set_file (GTK_FILE_CHOOSER (file_chooser_button_p),
                                   file_p,
                                   &error_p);
      ACE_ASSERT (result && !error_p);
      result =
        gtk_file_chooser_set_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
                                   file_p,
                                   &error_p);
      ACE_ASSERT (result && !error_p);
      g_object_unref (file_p); file_p = NULL;
      g_object_unref (file_2); file_2 = NULL;
    } // end IF
    else
    {
      result =
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                             filename_string.c_str ());
      ACE_ASSERT (result);
      result =
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_dialog_p),
                                             filename_string.c_str ());
      ACE_ASSERT (result);
      filename_string.clear ();
    } // end ELSE
  } // end IF
  else
  {
    result =
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                           Common_File_Tools::getTempDirectory ().c_str ());
    ACE_ASSERT (result);
    result =
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_dialog_p),
                                           Common_File_Tools::getTempDirectory ().c_str ());
    ACE_ASSERT (result);
  } // end ELSE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
      mode_2d =
        (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
#if defined (GTKGL_SUPPORT)
      mode_3d =
        (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzer3DMode;
#endif // GTKGL_SUPPORT
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
      mode_2d =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
#if defined (GTKGL_SUPPORT)
      mode_3d =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzer3DMode;
#endif // GTKGL_SUPPORT
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
  mode_2d =
    (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
#if defined (GTKGL_SUPPORT)
  mode_3d =
    (*modulehandler_configuration_iterator).second.second->spectrumAnalyzer3DMode;
#endif /* GTKGL_SUPPORT */
#endif // ACE_WIN32 || ACE_WIN64
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX));
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_3D_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                (mode_3d < STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX));

  drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
#if GTK_CHECK_VERSION (3,0,0)
//#if GTK_CHECK_VERSION(3,10,0)
//#else
  g_object_set (G_OBJECT (drawing_area_p),
                ACE_TEXT_ALWAYS_CHAR ("gtk-tooltip-timeout"), &tooltip_timeout,
                NULL);
//#endif // GTK_CHECK_VERSION (3,10,0)
#else
#if GTK_CHECK_VERSION(2,12,0) // *TODO*: this seems to be wrong
  g_object_set (G_OBJECT (drawing_area_p),
                ACE_TEXT_ALWAYS_CHAR ("gtk-tooltip-timeout"), &tooltip_timeout,
                NULL);
#endif // GTK_CHECK_VERSION (2,12,0)
#endif // GTK_CHECK_VERSION (3,0,0)

#if defined (GTKGL_SUPPORT)
  box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_DISPLAY_NAME)));
  ACE_ASSERT (box_p);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  gl_area_p = GTK_GL_AREA (gtk_gl_area_new ());
  ACE_ASSERT (gl_area_p);
  gtk_widget_realize (GTK_WIDGET (gl_area_p));
  gl_context_p = gtk_gl_area_get_context (gl_area_p);
  //ACE_ASSERT (gl_context_p);
  state_r.OpenGLContexts.insert (std::make_pair (gl_area_p, gl_context_p));
  opengl_contexts_iterator = state_r.OpenGLContexts.find (gl_area_p);

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
    goto error;
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
#else
#if defined (GTKGLAREA_SUPPORT)
  gl_area_p =
    GTK_GL_AREA (gtk_gl_area_new (gl_attributes_a));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_gl_area_new(), aborting\n")));
    goto error;
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
    goto error;
  } // end IF

  if (!gtk_widget_set_gl_capability (GTK_WIDGET (drawing_area_2), // widget
                                     gl_config_p,                 // configuration
                                     NULL,                        // share list
                                     true,                        // direct
                                     GDK_GL_RGBA_TYPE))           // render type
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_widget_set_gl_capability(): \"%m\", aborting\n")));
    goto error;
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
      //(*directshow_modulehandler_configuration_iterator).second.second->OpenGLWindow =
      //  (*opengl_contexts_iterator).first;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //(*mediafoundation_modulehandler_configuration_iterator).second.second->OpenGLWindow =
      //  (*opengl_contexts_iterator).first;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  //(*modulehandler_configuration_iterator).second.second->OpenGLWindow =
  //  (*opengl_contexts_iterator).first;
#endif // ACE_WIN32 || ACE_WIN64

#if GTK_CHECK_VERSION (3,0,0)
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
  gtk_widget_set_events (GTK_WIDGET ((*opengl_contexts_iterator).first),
                         GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,0,0) */

#if GTK_CHECK_VERSION (3,0,0)
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
  gtk_widget_set_visible (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */

  drawing_area_2 =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_3D_NAME)));
  ACE_ASSERT (drawing_area_2);
  //GList* children_p, *iterator_p;
  //children_p = gtk_container_get_children (GTK_CONTAINER (box_p));
  //for (iterator_p = children_p;
  //     iterator_p != NULL;
  //     iterator_p = g_list_next (iterator_p))
  //  if (GTK_WIDGET (iterator_p->data) == GTK_WIDGET (drawing_area_2))
  //  {
  //    gtk_widget_destroy (GTK_WIDGET (iterator_p->data));
  //    break;
  //  } // end IF
  gtk_widget_destroy (GTK_WIDGET (drawing_area_2));
  gtk_box_pack_start (box_p,
                      GTK_WIDGET ((*opengl_contexts_iterator).first),
                      TRUE, // expand
                      TRUE, // fill
                      0);   // padding
#if GTK_CHECK_VERSION (3,8,0)
//  gtk_builder_expose_object ((*iterator).second.second,
//                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GLAREA_3D_NAME),
//                             G_OBJECT ((*opengl_contexts_iterator).first));
#endif /* GTK_CHECK_VERSION (3,8,0) */
#endif /* GTKGL_SUPPORT */

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

  // step5: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step5a: connect default signals
  result_2 =
    g_signal_connect (G_OBJECT (dialog_p),
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      NULL);
  ACE_ASSERT (result_2);

  //--------------------------------------

#if defined (GTKGL_SUPPORT)
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("realize"),
                      G_CALLBACK (glarea_realize_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("unrealize"),
                      G_CALLBACK (glarea_unrealize_cb),
                      userData_in);
  ACE_ASSERT (result_2);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
 result_2 =
   g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                     ACE_TEXT_ALWAYS_CHAR ("create-context"),
                     G_CALLBACK (glarea_create_context_cb),
                     userData_in);
 ACE_ASSERT (result_2);
 result_2 =
   g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                     ACE_TEXT_ALWAYS_CHAR ("render"),
                     G_CALLBACK (glarea_render_cb),
                     userData_in);
 ACE_ASSERT (result_2);
 result_2 =
   g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                     ACE_TEXT_ALWAYS_CHAR ("resize"),
                     G_CALLBACK (glarea_resize_cb),
                     userData_in);
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
                      //ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                      ACE_TEXT_ALWAYS_CHAR ("draw"),
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
  ACE_ASSERT (result_2);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT

  //--------------------------------------

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  // debug info
#if defined (GTKGL_SUPPORT)
  ACE_ASSERT ((*opengl_contexts_iterator).first);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  error_p = gtk_gl_area_get_error ((*opengl_contexts_iterator).first);
  if (unlikely (error_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to realize OpenGL widget: \"%s\", continuing\n"),
                ACE_TEXT (error_p->message)));
    g_error_free (error_p); error_p = NULL;
    goto continue_3;
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
#if defined (_DEBUG)
  Common_UI_GTK_Tools::dumpGtkOpenGLInfo ((*opengl_contexts_iterator).first->glcontext);
#endif // _DEBUG
#else
#if defined (_DEBUG)
  Common_UI_GTK_Tools::dumpGtkOpenGLInfo ();
#endif // _DEBUG
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
#endif // GTKGL_SUPPORT

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,16,0)
continue_3:
#endif // GTK_CHECK_VERSION (3,16,0)
#endif // GTKGL_SUPPORT

  // step10: OpenGL context, ...
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
      goto error;
    }
  } // end SWITCH
#else
//  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second->OpenGLWindow);
  (*modulehandler_configuration_iterator).second.second->window =
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
      gtk_widget_get_window (GTK_WIDGET ((*opengl_contexts_iterator).first));
#else
      gtk_widget_get_window (GTK_WIDGET ((*opengl_contexts_iterator).first));
  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second->GdkWindow3D);
  (*modulehandler_configuration_iterator).second.second->GdkWindow3D =
    gtk_widget_get_window (GTK_WIDGET (&(*opengl_contexts_iterator).first->darea));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->GdkWindow3D);
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
    (*opengl_contexts_iterator).first->glcontext;
  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second->GdkWindow3D);
  (*modulehandler_configuration_iterator).second.second->GdkWindow3D =
    gtk_widget_get_window (GTK_WIDGET (&(*opengl_contexts_iterator).first->darea));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->GdkWindow3D);
#else
    gl_context_p;
  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second->GdkWindow3D);
  (*modulehandler_configuration_iterator).second.second->GdkWindow3D =
    gtk_widget_get_gl_drawable (GTK_WIDGET (drawing_area_2));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->GdkWindow3D);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->window);
#endif // ACE_WIN32 || ACE_WIN64
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

  // step11: activate some widgets
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
  gtk_combo_box_set_active (combo_box_p, index_i);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID effect_id = GUID_NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      effect_id =
        (*directshow_modulehandler_configuration_iterator).second.second->effect;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      effect_id =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->effect;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
  is_active_b = !InlineIsEqualGUID (effect_id, GUID_NULL);
#else
  is_active_b =
    !(*modulehandler_configuration_iterator).second.second->effect.empty ();
#endif // ACE_WIN32 || ACE_WIN64
  if (is_active_b)
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
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), (n_rows > 0));

    // *TODO*: there must be a better way to do this...
    GtkTreeIter tree_iterator;
    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store_p),
                                        &tree_iterator))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_tree_model_get_iter_first(), aborting\n")));
      goto error;
    } // end IF
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
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
      struct _GUID effect_id_2 = Common_OS_Tools::StringToGUID (effect_string_2);
      if (InlineIsEqualGUID (effect_id_2, GUID_NULL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                    ACE_TEXT (effect_string_2.c_str ())));
        goto error;
      } // end IF
      if (InlineIsEqualGUID (effect_id, effect_id_2))
#else
      if ((*modulehandler_configuration_iterator).second.second->effect == effect_string_2)
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
    gtk_toggle_button_set_active (toggle_button_p, TRUE);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      is_active_b =
        (*directshow_modulehandler_configuration_iterator).second.second->mute;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      is_active_b =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->mute;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  is_active_b = (*modulehandler_configuration_iterator).second.second->mute;
#endif // ACE_WIN32 || ACE_WIN64
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_MUTE_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p, is_active_b);

  if (!filename_string.empty ())
  {
    toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_SAVE_NAME)));
    ACE_ASSERT (toggle_button_p);

#if GTK_CHECK_VERSION (3,0,0)
    g_signal_handlers_block_by_func (G_OBJECT (toggle_button_p),
                                     (gpointer)G_CALLBACK (togglebutton_save_toggled_cb),
                                     userData_in);
#elif GTK_CHECK_VERSION (2,0,0)
    gtk_signal_handler_block_by_func (GTK_OBJECT (toggle_button_p),
                                      G_CALLBACK (togglebutton_save_toggled_cb),
                                      userData_in);
#endif // GTK_CHECK_VERSION (x,0,0)
    gtk_toggle_button_set_active (toggle_button_p, TRUE);
#if GTK_CHECK_VERSION (3,0,0)
    g_signal_handlers_unblock_by_func (G_OBJECT (toggle_button_p),
                                       (gpointer)G_CALLBACK (togglebutton_save_toggled_cb),
                                       userData_in);
#elif GTK_CHECK_VERSION (2,0,0)
    gtk_signal_handler_unblock_by_func (GTK_OBJECT (toggle_button_p),
                                        G_CALLBACK (togglebutton_save_toggled_cb),
                                        userData_in);
#endif // GTK_CHECK_VERSION (x,0,0)
  } // end IF

  is_active_b =
    ((mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX) ||
     (mode_3d < STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX));
  if (is_active_b)
  {
    toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME)));
    ACE_ASSERT (toggle_button_p);
    gtk_toggle_button_set_active (toggle_button_p, TRUE);

    if (mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX)
    {
      radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    (mode_2d == STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE) ? ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME)
                                                                                                                        : ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SPECTRUM_NAME)));
      ACE_ASSERT (radio_button_p);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p), TRUE);
    } // end IF
    if (mode_3d < STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX)
    {
      toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_3D_NAME)));
      ACE_ASSERT (toggle_button_p);
      gtk_toggle_button_set_active (toggle_button_p, TRUE);
    } // end IF
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (!(*directshow_modulehandler_configuration_iterator).second.second->window);
      (*directshow_modulehandler_configuration_iterator).second.second->window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->window);
      window_p =
        (*directshow_modulehandler_configuration_iterator).second.second->window;
      (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->window =
        window_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (!(*mediafoundation_modulehandler_configuration_iterator).second.second->window);
      (*mediafoundation_modulehandler_configuration_iterator).second.second->window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->window);
      window_p =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->window;
      (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->window =
        window_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
//  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second->window);
  (*modulehandler_configuration_iterator).second.second->window =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->window);
  window_p = (*modulehandler_configuration_iterator).second.second->window;
  (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->window =
    window_p;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (window_p);

  return G_SOURCE_REMOVE;

error:
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
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
      return G_SOURCE_REMOVE;
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
                    false, // recurse upstream ?
                    true); // high priority ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (ui_cb_data_base_p->COMInitialized)
  {
    CoUninitialize ();
    ui_cb_data_base_p->COMInitialized = false;
  } // end IF
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

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // *IMPORTANT NOTE*: there are three major reasons for being here that are not
  //                   mutually exclusive:
  //                   - user pressed stop
  //                   - audio file has ended playing
  //                   - there was an error on the stream --> abort

  ui_data_base_p->spectrumAnalyzerCBData.dispatch = NULL;

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p), GTK_STOCK_MEDIA_RECORD);
  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    untoggling_record_button = true;
    gtk_toggle_button_set_active (toggle_button_p, FALSE);
    //gtk_signal_emit_by_name (GTK_OBJECT (toggle_button_p),
    //                         ACE_TEXT_ALWAYS_CHAR ("toggled"),
    //                         userData_in);
  } // end IF
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), TRUE);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_DEVICE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                            ui_data_base_p->switchCaptureDevice);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_FILE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_DEVICE_SETTINGS_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_DEVICE_RESET_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_VBOX_FORMAT_OPTIONS_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), TRUE);

  box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_EFFECT_2_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), TRUE);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_PROPERTIES_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
      static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

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
                                     static_cast<gdouble> (ui_cb_data_base_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          break;
        }
        case COMMON_UI_EVENT_ABORT:
        case COMMON_UI_EVENT_FINISHED:
        case COMMON_UI_EVENT_STEP:
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
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDSAMPLES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_base_p->progressData.statistic.capturedFrames));
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDSAMPLES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_base_p->progressData.statistic.droppedFrames));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("statistic analysis: a/s/v (a/v): %.2f/%.2f; (a/c/v): %.2f/%u/%.2f; (a/v): %.2f/%.2f\n"),
                      ui_cb_data_base_p->progressData.statistic.amplitudeAverage,
                      ui_cb_data_base_p->progressData.statistic.amplitudeVariance,
                      ui_cb_data_base_p->progressData.statistic.streakAverage,
                      ui_cb_data_base_p->progressData.statistic.streakCount,
                      ui_cb_data_base_p->progressData.statistic.streakVariance,
                      ui_cb_data_base_p->progressData.statistic.volumeAverage,
                      ui_cb_data_base_p->progressData.statistic.volumeVariance));

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

  // display renderer statistics
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      if ((directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW) ||
          !directshow_ui_cb_data_p->stream->isRunning ())
        break;
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->builder);
      Stream_MediaFrameWork_DirectSound_Statistics_t statistics_a;
      Stream_MediaFramework_DirectShow_Tools::getAudioRendererStatistics ((*directshow_modulehandler_configuration_iterator).second.second->builder,
                                                                          statistics_a);
      //for (Stream_MediaFrameWork_DirectSound_StatisticsIterator_t iterator_2 = statistics_a.begin ();
      //     iterator_2 != statistics_a.end ();
      //     ++iterator_2)
      //{
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("renderer statistic: %s: %d/%d\n"),
      //              ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString ((*iterator_2).first).c_str ()),
      //              (*iterator_2).second.first, (*iterator_2).second.second));
      //} // end FOR
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("\n")));

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_ProgressData* data_p =
      static_cast<struct Test_U_AudioEffect_ProgressData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->state);
  Common_UI_GTK_BuildersConstIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->state->builders.end ());
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  int result = -1;
  ACE_THR_FUNC_RETURN exit_status;
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  bool done_b = false;
  std::ostringstream converter;
  ACE_Time_Value elapsed, now = COMMON_TIME_NOW;
  ACE_UINT32 bytes_per_second_i = 0;
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  static ACE_Time_Value previous_timestamp = now;
  static ACE_UINT64 previous_bytes_i = 0;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);

  for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
       iterator_3 != data_p->completedActions.end ();
       ++iterator_3)
  {
    iterator_2 = data_p->pendingActions.find (*iterator_3);
    ACE_ASSERT (iterator_2 != data_p->pendingActions.end ());
    result = thread_manager_p->join ((*iterator_2).second.id (),
                                     &exit_status);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%u): \"%m\", continuing\n"),
                  (*iterator_2).second.id ()));
    else
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: %u)...\n"),
                  (*iterator_2).second.id (),
                  exit_status));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: %@)...\n"),
                  (*iterator_2).second.id (),
                  exit_status));
#endif // ACE_WIN32 || ACE_WIN64
    } // end ELSE

    data_p->state->eventSourceIds.erase (*iterator_3);
    data_p->pendingActions.erase (iterator_2);
  } // end FOR
  data_p->completedActions.clear ();

  if (unlikely (data_p->pendingActions.empty ()))
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

    done_b = true;

    result = data_p->state->condition.broadcast ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));
  } // end IF

  // calculate fps
  elapsed = now - previous_timestamp;
  bytes_per_second_i =
    static_cast<ACE_UINT32> (static_cast<double> (data_p->statistic.bytes - previous_bytes_i) * (1000 / (double)elapsed.msec ()));
  previous_timestamp = now;
  previous_bytes_i = data_p->statistic.bytes;
  converter <<
    static_cast<ACE_UINT32> (bytes_per_second_i / static_cast<double> (data_p->bytesPerFrame));
  converter << ACE_TEXT_ALWAYS_CHAR (" fps");
  gtk_progress_bar_set_text (progress_bar_p,
                             (done_b ? ACE_TEXT_ALWAYS_CHAR ("")
                                     : converter.str ().c_str ()));
  gtk_progress_bar_pulse (progress_bar_p);

  // reschedule ?
  return (done_b ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

gboolean
idle_update_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_display_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // trigger refresh of the 2D area ?
  GtkDrawingArea* drawing_area_p = NULL;
  GdkWindow* window_p = NULL;
  if (!ui_cb_data_base_p->render2d)
    goto continue_2;
  drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  if (unlikely (!window_p))
    goto continue_2; // <-- not realized yet

  gdk_window_invalidate_rect (window_p,
                              NULL,   // whole window
                              FALSE); // invalidate children ?

continue_2:
#if defined (GTKGL_SUPPORT)
  // trigger refresh of the 3D OpenGL area ?
  if (!ui_cb_data_base_p->render3d)
    return G_SOURCE_CONTINUE;

  ACE_ASSERT (!state_r.OpenGLContexts.empty ());
  Common_UI_GTK_GLContextsIterator_t iterator_2 = state_r.OpenGLContexts.begin ();
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  window_p = gtk_widget_get_window (GTK_WIDGET ((*iterator_2).first));
#else
  window_p = gtk_widget_get_window (GTK_WIDGET (&(*iterator_2).first->darea));
#endif // GTK_CHECK_VERSION (3,16,0)
#else
  window_p = gtk_widget_get_window (GTK_WIDGET (&(*iterator_2).first->darea));
#endif // GTK_CHECK_VERSION
  if (unlikely (!window_p))
    goto continue_3; // <-- not realized yet

  gdk_window_invalidate_rect (window_p,
                              NULL,
                              FALSE);
continue_3:
#endif /* GTKGL_SUPPORT */
  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
togglebutton_record_toggled_cb (GtkToggleButton* toggleButton_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_record_toggled_cb"));

  // handle untoggle --> RECORD
  if (untoggling_record_button)
  {
    untoggling_record_button = false;
    return; // done
  } // end IF

  // --> user pressed play/pause/stop

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
#endif // ACE_WIN32 || ACE_WIN64
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  Stream_IStreamControlBase* stream_p = NULL;
  bool is_file_source_b = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);

      stream_p = directshow_ui_cb_data_p->stream;
      use_framework_source_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW);
      is_file_source_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType == AUDIOEFFECT_SOURCE_FILE);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);

      stream_p = mediafoundation_ui_cb_data_p->stream;
      use_framework_source_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION);
      is_file_source_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType == AUDIOEFFECT_SOURCE_FILE);
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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  stream_p = ui_cb_data_p->stream;
  is_file_source_b =
    (ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType == AUDIOEFFECT_SOURCE_FILE);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  // toggle ?
  if (!gtk_toggle_button_get_active (toggleButton_in))
  {
    // --> user pressed pause/stop

    // step0: modify widgets
    // *NOTE*: wait for "end of session"
    gtk_widget_set_sensitive (GTK_WIDGET (toggleButton_in), FALSE);

    // step1: stop stream
    stream_p->stop (false,             // wait for completion ?
                    false,             // recurse upstream ?
                    is_file_source_b); // high priority ?

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

    return;
  } // end IF

  // --> user pressed record

  struct Test_U_GTK_ThreadData* thread_data_p = NULL;
  char thread_name[BUFSIZ];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;

  GtkSpinButton* spin_button_p = NULL;
  unsigned int value_i = 0;
  ACE_UINT32 bytes_per_frame_i = 0;

  if (ui_cb_data_base_p->isFirst)
    ui_cb_data_base_p->isFirst = false;

  // step0: update configuration
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  value_i =
    static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));
  ACE_ASSERT (value_i);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p->configuration->allocatorProperties.cbBuffer =
        static_cast<long> (value_i);
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
    value_i;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      bytes_per_frame_i =
        (audio_info_header_p->wBitsPerSample / 8) * audio_info_header_p->nChannels;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      UINT32 bits_per_sample_i = 0, channels_i = 0;
      HRESULT result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                                            &bits_per_sample_i);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                            &channels_i);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      bytes_per_frame_i = (bits_per_sample_i / 8) * channels_i;
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
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->ALSAConfiguration);
  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second->ALSAConfiguration->format);
  (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->format =
    &ui_cb_data_p->configuration->streamConfiguration.configuration_->format;

  if (ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType == AUDIOEFFECT_SOURCE_DEVICE)
  { ACE_ASSERT (ui_cb_data_p->handle);
    if (!Stream_MediaFramework_ALSA_Tools::setFormat (ui_cb_data_p->handle,
                                                      *(*modulehandler_configuration_iterator).second.second->ALSAConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::setFormat(): \"%m\", returning\n")));
      (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->format = NULL;
      return;
    } // end IF
  } // end IF
  (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->format =
    NULL;

  bytes_per_frame_i =
    (snd_pcm_format_width (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format) / 8) *
    ui_cb_data_p->configuration->streamConfiguration.configuration_->format.channels;
#endif // ACE_WIN32 || ACE_WIN64

  // step2: modify widgets
  gtk_button_set_label (GTK_BUTTON (toggleButton_in), GTK_STOCK_MEDIA_STOP);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_DEVICE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_FILE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_VBOX_FORMAT_OPTIONS_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), FALSE);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_DEVICE_SETTINGS_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_DEVICE_RESET_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_EFFECT_2_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), FALSE);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_PROPERTIES_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  // step1: set up progress reporting
  ui_cb_data_base_p->progressData.bytesPerFrame = bytes_per_frame_i;
  ui_cb_data_base_p->progressData.statistic = Test_U_AudioEffect_Statistic ();
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  GtkAllocation allocation_s;
  gtk_widget_get_allocation (GTK_WIDGET (progress_bar_p),
                             &allocation_s);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (allocation_s.width));
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
  ACE_OS::memset (thread_name, 0, sizeof (char[BUFSIZ]));
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_OS::strncpy (thread_name,
                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME),
                     std::min (static_cast<size_t> (BUFSIZ - 1), static_cast<size_t> (ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME)))));
#else
  ACE_ASSERT (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH <= BUFSIZ);
  ACE_OS::strncpy (thread_name,
                   ACE_TEXT (TEST_U_STREAM_THREAD_NAME),
                   std::min (static_cast<size_t> (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH - 1), static_cast<size_t> (ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_THREAD_NAME)))));
#endif // ACE_WIN32 || ACE_WIN64
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
      g_idle_add (idle_update_progress_cb,
                  &ui_cb_data_base_p->progressData);
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
    // *NOTE*: do not add it to the UI state, as those will be cleared on stop()
    //state_r.eventSourceIds.insert (ui_cb_data_base_p->progressData.eventSourceId);
  } // end lock scope

  // step12: initialize updates
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    // schedule asynchronous updates of the info view
    guint event_source_id =
      g_idle_add (idle_update_info_display_cb,
                  userData_in);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(): \"%m\", continuing\n")));

    event_source_id = g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_CAIRO_MS / 2, // ~60fps
                                     idle_update_display_cb,
                                     userData_in);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", continuing\n")));
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
} // button_cut_clicked_cb

void
button_properties_clicked_cb (GtkButton* button_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_properties_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
  //GtkDrawingArea* drawing_area_p =
  //  GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  //ACE_ASSERT (drawing_area_p);
  //GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  //ACE_ASSERT (window_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (ui_cb_data_base_p);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      if (!(*directshow_modulehandler_configuration_iterator).second.second->builder)
        break; // not using DirectShow
      IBaseFilter* filter_p = NULL;
      HRESULT result =
        (*directshow_modulehandler_configuration_iterator).second.second->builder->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO_L,
                                                                                                     &filter_p);
      if (FAILED (result))
        break; // using Null renderer ?
      ACE_ASSERT (filter_p);
      ISpecifyPropertyPages* property_pages_p = NULL;
      result = filter_p->QueryInterface (IID_PPV_ARGS (&property_pages_p));
      ACE_ASSERT (SUCCEEDED (result) && property_pages_p);
      struct tagCAUUID uuids_a;
      ACE_OS::memset (&uuids_a, 0, sizeof (struct tagCAUUID));
      result = property_pages_p->GetPages (&uuids_a);
      ACE_ASSERT (SUCCEEDED (result) && uuids_a.pElems);
      property_pages_p->Release (); property_pages_p = NULL;
      IUnknown* iunknown_p = NULL;
      filter_p->QueryInterface (IID_PPV_ARGS (&iunknown_p));
      ACE_ASSERT (SUCCEEDED (result) && iunknown_p);
      //ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
      LCID locale_id = 1033;
      // display modal properties dialog
      // *TODO*: implement modeless support (i.e. return immediately)
      result =
        OleCreatePropertyFrame (NULL,//gdk_win32_window_get_impl_hwnd (window_p), // Parent window {NULL ? modeless : modal}
                                0, 0,                     // Reserved
#if defined (OLE2ANSI)
                                Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str (), // Caption for the dialog box
#else
                                ACE_TEXT_ALWAYS_WCHAR (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()), // Caption for the dialog box
#endif // OLE2ANSI
                                1,                        // Number of objects (just the filter)
                                &iunknown_p,              // Array of object pointers
                                uuids_a.cElems,           // Number of property pages
                                uuids_a.pElems,           // Array of property page CLSIDs
                                locale_id,                // Locale identifier
                                0, NULL);                 // Reserved
      ACE_ASSERT (SUCCEEDED (result));
      iunknown_p->Release (); iunknown_p = NULL;
      CoTaskMemFree (uuids_a.pElems); uuids_a.pElems = NULL;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (ui_cb_data_base_p);
      ACE_UNUSED_ARG (mediafoundation_ui_cb_data_p);
      //ACE_UNUSED_ARG (window_p);
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
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (ui_cb_data_base_p);
  ACE_UNUSED_ARG (ui_cb_data_p);
  //ACE_UNUSED_ARG (window_p);
#endif // ACE_WIN32 || ACE_WIN64
}

void
button_report_clicked_cb (GtkButton* button_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_report_clicked_cb"));

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
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);

  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  char* filename_p = NULL;
  GFile* file_p =
    gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_dialog_p));
  if (!file_p)
    goto continue_;
  filename_p = g_file_get_path (file_p);
  if (!filename_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));
    g_object_unref (file_p); file_p = NULL;
    return;
  } // end IF
  g_object_unref (file_p); file_p = NULL;
continue_:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      if (is_active)
        (*directshow_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
          Common_UI_GTK_Tools::UTF8ToLocale ((filename_p ? filename_p : ACE_TEXT_ALWAYS_CHAR ("")), -1);
      else
        (*directshow_modulehandler_configuration_iterator).second.second->fileIdentifier.clear ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      if (is_active)
        (*mediafoundation_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
          Common_UI_GTK_Tools::UTF8ToLocale ((filename_p ? filename_p : ACE_TEXT_ALWAYS_CHAR ("")), -1);
      else
        (*mediafoundation_modulehandler_configuration_iterator).second.second->fileIdentifier.clear ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      break;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  if (is_active)
    (*modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
      Common_UI_GTK_Tools::UTF8ToLocale ((filename_p ? filename_p : ACE_TEXT_ALWAYS_CHAR ("")), -1);
  else
    (*modulehandler_configuration_iterator).second.second->fileIdentifier.clear ();
#endif // ACE_WIN32 || ACE_WIN64
  g_free (filename_p);

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (file_chooser_button_p),
                            is_active);
} // togglebutton_save_toggled_cb

void
hscale_device_volume_value_changed_cb (GtkRange* range_in,
                                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_device_volume_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->captureVolumeControl);
      HRESULT result =
        directshow_ui_cb_data_p->captureVolumeControl->SetMasterVolumeLevelScalar (static_cast<float> (gtk_range_get_value (range_in) / 100.0),
                                                                                   NULL);
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->captureVolumeControl);
      HRESULT result =
        mediafoundation_ui_cb_data_p->captureVolumeControl->SetMasterVolumeLevelScalar (static_cast<float> (gtk_range_get_value (range_in) / 100.0),
                                                                                        NULL);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  if (!Stream_MediaFramework_ALSA_Tools::setVolumeLevel ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_VOLUME_NAME),
                                                         true, // capture
                                                         static_cast<long> (gtk_range_get_value (range_in))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::setVolumeLevel(\"%s\",\"%s\",%d), returning\n"),
                 ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ()),
                 ACE_TEXT (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_VOLUME_NAME),
                 static_cast<long> (gtk_range_get_value (range_in))));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
} // hscale_device_volume_value_changed_cb

gboolean
hscale_device_boost_change_value_cb (GtkRange* range_in,
                                     GtkScrollType* scrollType_in,
                                     gdouble value_in,
                                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_device_boost_change_value_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      if (!directshow_ui_cb_data_p->boostControl)
        break;
      float min_level_f = 0.0F, max_level_f = 0.0F, stepping_f = 0.0F;
      HRESULT result =
        directshow_ui_cb_data_p->boostControl->GetLevelRange (0,
                                                              &min_level_f,
                                                              &max_level_f,
                                                              &stepping_f);
      ACE_ASSERT (SUCCEEDED (result));
      // determine the closest discrete value
      std::vector<float> values_a;
      for (float i = min_level_f;
           i <= max_level_f;
           i += stepping_f)
        values_a.push_back (i);
      std::vector<float>::const_iterator iterator =
        std::find (values_a.begin (), values_a.end (),
                   static_cast<float> (value_in));
      if (iterator != values_a.end ())
        return FALSE; // propagate the event
      iterator =
        std::lower_bound (values_a.begin (), values_a.end (),
                          static_cast<float> (value_in));
      gdouble value_f = values_a.back ();
      if (iterator != values_a.end ())
        value_f = *iterator;
      g_signal_emit_by_name (G_OBJECT (range_in),
                             ACE_TEXT_ALWAYS_CHAR ("change-value"),
                             scrollType_in, static_cast<gdouble> (value_f), userData_in,
                             NULL);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      if (!mediafoundation_ui_cb_data_p->boostControl)
        break;
      float min_level_f = 0.0F, max_level_f = 0.0F, stepping_f = 0.0F;
      HRESULT result =
        mediafoundation_ui_cb_data_p->boostControl->GetLevelRange (0,
                                                                   &min_level_f,
                                                                   &max_level_f,
                                                                   &stepping_f);
      ACE_ASSERT (SUCCEEDED (result));
      // determine the closest discrete value
      std::vector<float> values_a;
      for (float i = min_level_f;
           i <= max_level_f;
           i += stepping_f)
        values_a.push_back (i);
      std::vector<float>::const_iterator iterator =
        std::find (values_a.begin (), values_a.end (),
                   static_cast<float> (value_in));
      if (iterator != values_a.end ())
        return FALSE; // propagate the event
      iterator =
        std::lower_bound (values_a.begin (), values_a.end (),
                          static_cast<float> (value_in));
      gdouble value_f = values_a.back ();
      if (iterator != values_a.end ())
        value_f = *iterator;
      g_signal_emit_by_name (G_OBJECT (range_in),
                             ACE_TEXT_ALWAYS_CHAR ("change-value"),
                             scrollType_in, static_cast<gdouble> (value_f), userData_in,
                             NULL);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      break;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  long min_level_i = 0, max_level_i = 0, current_level_i = 0;
  if (!Stream_MediaFramework_ALSA_Tools::getVolumeLevels ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME),
                                                          true, // capture
                                                          min_level_i,
                                                          max_level_i,
                                                          current_level_i))
  {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getVolumeLevels(\"%s\",\"%s\"), returning\n"),
                 ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ()),
                 ACE_TEXT (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME)));
      return TRUE;
  } // end IF
  // determine the closest discrete value
  std::vector<long> values_a;
  for (long i = min_level_i;
       i <= max_level_i;
       ++i)
      values_a.push_back (i);
  std::vector<long>::const_iterator iterator =
      std::find (values_a.begin (), values_a.end (),
                 static_cast<long> (value_in));
  if (iterator != values_a.end ())
      return FALSE; // propagate the event
  iterator =
      std::lower_bound (values_a.begin (), values_a.end (),
                       static_cast<long> (value_in));
  long value_i = values_a.back ();
  if (iterator != values_a.end ())
      value_i = *iterator;
  g_signal_emit_by_name (G_OBJECT (range_in),
                         ACE_TEXT_ALWAYS_CHAR ("change-value"),
                         scrollType_in, static_cast<gdouble> (value_i), userData_in,
                         NULL);
#endif // ACE_WIN32 || ACE_WIN64

  return TRUE;
} // hscale_device_boost_change_value_cb

void
hscale_device_boost_value_changed_cb (GtkRange* range_in,
                                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_device_boost_value_changed_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

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
      ACE_ASSERT (directshow_ui_cb_data_p->boostControl);
      float value_f = static_cast<float> (gtk_range_get_value (range_in));
      UINT num_channels_i =
        Stream_MediaFramework_DirectShow_Tools::toChannels (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      HRESULT result = E_FAIL;
      for (UINT i = 0;
           i < num_channels_i;
           ++i)
      {
        result = directshow_ui_cb_data_p->boostControl->SetLevel (i,
                                                                  value_f,
                                                                  NULL);
        //ACE_ASSERT (SUCCEEDED (result));
      } // end FOR
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      if (!mediafoundation_ui_cb_data_p->boostControl)
        break;
      float value_f = static_cast<float> (gtk_range_get_value (range_in));
      UINT32 num_channels_i = 0;
      HRESULT result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                            &num_channels_i);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      for (UINT i = 0;
           i < static_cast<UINT> (num_channels_i);
           ++i)
      {
        result = mediafoundation_ui_cb_data_p->boostControl->SetLevel (i,
                                                                       value_f,
                                                                       NULL);
        //ACE_ASSERT (SUCCEEDED (result));
      } // end FOR
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

  if (!Stream_MediaFramework_ALSA_Tools::setVolumeLevel ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME),
                                                         true, // capture
                                                         static_cast<long> (gtk_range_get_value (range_in))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::setVolumeLevel(\"%s\",\"%s\",%d), returning\n"),
                ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ()),
                ACE_TEXT (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME),
                static_cast<long> (gtk_range_get_value (range_in))));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
} // hscale_device_boost_value_changed_cb

void
hscale_volume_value_changed_cb (GtkRange* range_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_volume_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->renderVolumeControl);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      HRESULT result =
        directshow_ui_cb_data_p->renderVolumeControl->SetMasterVolume (static_cast<float> (gtk_range_get_value (range_in) / 100.0),
                                                                       NULL);
      ACE_ASSERT (SUCCEEDED (result));
      (*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration->amplitude =
        gtk_range_get_value (range_in) / 100.0;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->renderVolumeControl);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      HRESULT result =
        mediafoundation_ui_cb_data_p->renderVolumeControl->SetMasterVolume (static_cast<float> (gtk_range_get_value (range_in) / 100.0),
                                                                            NULL);
      ACE_ASSERT (SUCCEEDED (result));
      (*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration->amplitude =
        gtk_range_get_value (range_in) / 100.0;
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->generatorConfiguration);

  if (!Stream_MediaFramework_ALSA_Tools::setVolumeLevel ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_PLAYBACK_DEFAULT_SELEM_VOLUME_NAME),
                                                         false, // playback
                                                         static_cast<long> (gtk_range_get_value (range_in))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::setVolumeLevel(\"%s\",\"%s\",%d), returning\n"),
                 ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ()),
                 ACE_TEXT (STREAM_LIB_ALSA_PLAYBACK_DEFAULT_SELEM_VOLUME_NAME),
                 static_cast<long> (gtk_range_get_value (range_in))));
    return;
  } // end IF
  (*modulehandler_configuration_iterator).second.second->generatorConfiguration->amplitude =
    gtk_range_get_value (range_in) / 100.0;
#endif // ACE_WIN32 || ACE_WIN64
} // hscale_volume_value_changed_cb

void
hscale_amplitude_value_changed_cb (GtkRange* range_in,
                                   gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_amplitude_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      (*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration->amplitude =
        gtk_range_get_value (range_in);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      (*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration->amplitude =
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->generatorConfiguration);

  (*modulehandler_configuration_iterator).second.second->generatorConfiguration->amplitude =
    gtk_range_get_value (range_in);
#endif // ACE_WIN32 || ACE_WIN64
} // hscale_sinus_amplitude_value_changed_cb

void
hscale_frequency_value_changed_cb (GtkRange* range_in,
                                   gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_frequency_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      (*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration->frequency =
        gtk_range_get_value (range_in);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      (*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration->frequency =
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->generatorConfiguration);

  (*modulehandler_configuration_iterator).second.second->generatorConfiguration->frequency =
    gtk_range_get_value (range_in);
#endif // ACE_WIN32 || ACE_WIN64
} // hscale_frequency_value_changed_cb

void
hscale_perlin_frequency_value_changed_cb (GtkRange* range_in,
                                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_perlin_frequency_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStream_t* istream_p = NULL;
  const Stream_Module_t* module_p = NULL;
#if defined (LIBNOISE_SUPPORT)
  Common_IGetR_3_T<noise::module::Perlin>* iget_p = NULL;
#endif // LIBNOISE_SUPPORT
#if defined(ACE_WIN32) || defined(ACE_WIN64)
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
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
  struct Test_U_AudioEffect_UI_CBData* cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->stream);
  istream_p = dynamic_cast<Stream_IStream_t*> (cb_data_p->stream);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (istream_p);

#if defined (LIBNOISE_SUPPORT)
  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  iget_p =
    dynamic_cast<Common_IGetR_3_T<noise::module::Perlin>*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (iget_p);
  noise::module::Perlin& module_r =
    const_cast<noise::module::Perlin&> (iget_p->getR_3 ());
  module_r.SetFrequency (gtk_range_get_value (range_in));
#endif // LIBNOISE_SUPPORT
} // hscale_perlin_frequency_value_changed_cb

void
hscale_perlin_octaves_value_changed_cb (GtkRange* range_in,
                                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_perlin_octaves_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStream_t* istream_p = NULL;
  const Stream_Module_t* module_p = NULL;
#if defined (LIBNOISE_SUPPORT)
  Common_IGetR_3_T<noise::module::Perlin>* iget_p = NULL;
#endif // LIBNOISE_SUPPORT
#if defined(ACE_WIN32) || defined(ACE_WIN64)
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
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
  struct Test_U_AudioEffect_UI_CBData* cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->stream);
  istream_p = dynamic_cast<Stream_IStream_t*> (cb_data_p->stream);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (istream_p);

#if defined (LIBNOISE_SUPPORT)
  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  iget_p =
    dynamic_cast<Common_IGetR_3_T<noise::module::Perlin>*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (iget_p);
  noise::module::Perlin& module_r =
    const_cast<noise::module::Perlin&> (iget_p->getR_3 ());
  module_r.SetOctaveCount (static_cast<int> (gtk_range_get_value (range_in)));
#endif // LIBNOISE_SUPPORT
} // hscale_perlin_octaves_value_changed_cb

void
hscale_perlin_persistence_value_changed_cb (GtkRange* range_in,
                                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_perlin_persistence_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStream_t* istream_p = NULL;
  const Stream_Module_t* module_p = NULL;
#if defined (LIBNOISE_SUPPORT)
  Common_IGetR_3_T<noise::module::Perlin>* iget_p = NULL;
#endif // LIBNOISE_SUPPORT
#if defined(ACE_WIN32) || defined(ACE_WIN64)
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
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
  struct Test_U_AudioEffect_UI_CBData* cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->stream);
  istream_p = dynamic_cast<Stream_IStream_t*> (cb_data_p->stream);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (istream_p);

#if defined (LIBNOISE_SUPPORT)
  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  iget_p =
    dynamic_cast<Common_IGetR_3_T<noise::module::Perlin>*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (iget_p);
  noise::module::Perlin& module_r =
    const_cast<noise::module::Perlin&> (iget_p->getR_3 ());
  module_r.SetPersistence (gtk_range_get_value (range_in));
#endif // LIBNOISE_SUPPORT
} // hscale_perlin_persistence_value_changed_cb

void
hscale_perlin_lacunarity_value_changed_cb (GtkRange* range_in,
                                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_perlin_lacunarity_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStream_t* istream_p = NULL;
  const Stream_Module_t* module_p = NULL;
#if defined (LIBNOISE_SUPPORT)
  Common_IGetR_3_T<noise::module::Perlin>* iget_p = NULL;
#endif // LIBNOISE_SUPPORT
#if defined(ACE_WIN32) || defined(ACE_WIN64)
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
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
  struct Test_U_AudioEffect_UI_CBData* cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->stream);
  istream_p = dynamic_cast<Stream_IStream_t*> (cb_data_p->stream);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (istream_p);

#if defined (LIBNOISE_SUPPORT)
  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  iget_p =
    dynamic_cast<Common_IGetR_3_T<noise::module::Perlin>*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (iget_p);
  noise::module::Perlin& module_r =
    const_cast<noise::module::Perlin&> (iget_p->getR_3 ());
  module_r.SetLacunarity (gtk_range_get_value (range_in));
#endif // LIBNOISE_SUPPORT
} // hscale_perlin_lacunarity_value_changed_cb

#if defined (ACE_WIN32) || defined (ACE_WIN64)
void
hscale_win32_ds_flanger_wetdrymix_value_changed_cb (GtkRange* range_in,
                                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_win32_ds_flanger_wetdrymix_value_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
  ACE_ASSERT (directshow_ui_cb_data_p);
  ACE_ASSERT (directshow_ui_cb_data_p->configuration);
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator =
    directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
  (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.flangerOptions.fWetDryMix =
    static_cast<FLOAT> (gtk_range_get_value (range_in));
  if (!(*directshow_modulehandler_configuration_iterator).second.second->builder)
    return; // stream not running ?
  IBaseFilter* filter_p = NULL;
  HRESULT result =
    (*directshow_modulehandler_configuration_iterator).second.second->builder->FindFilterByName (STREAM_DEC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO_L,
                                                                                                 &filter_p);
  if (FAILED (result) || !filter_p)
    return; // stream not running ?
  IDirectSoundFXFlanger* effect_p = NULL;
  result = filter_p->QueryInterface (IID_IDirectSoundFXFlanger,
                                     (void**)&effect_p);
  ACE_ASSERT (SUCCEEDED (result) && effect_p);
  filter_p->Release (); filter_p = NULL;
  result =
    effect_p->SetAllParameters (&(*directshow_modulehandler_configuration_iterator).second.second->effectOptions.flangerOptions);
  ACE_ASSERT (SUCCEEDED (result));
  effect_p->Release (); effect_p = NULL;
} // hscale_win32_ds_flanger_wetdrymix_value_changed_cb
#endif // ACE_WIN32 || ACE_WIN64

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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active_b = gtk_toggle_button_get_active (toggleButton_in);
  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_EFFECT_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                            is_active_b);

  std::string effect_string;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
#endif // ACE_WIN32 || ACE_WIN64
  if (is_active_b)
  {
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
#endif // GTK_CHECK_VERSION (2,30,0)
      gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                                &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                1, &value);
#else
                                0, &value);
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
      effect_string = g_value_get_string (&value);
      g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      GUID_s = Common_OS_Tools::StringToGUID (effect_string);
      if (InlineIsEqualGUID (GUID_s, GUID_NULL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                    ACE_TEXT (effect_string.c_str ())));
        return;
      } // end IF
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator).second.second->effect =
        GUID_s;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second->effect =
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
  (*modulehandler_configuration_iterator).second.second->effect =
    effect_string;
#endif // ACE_WIN32 || ACE_WIN64
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
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*directshow_modulehandler_configuration_iterator).second.second->mute =
        is_active;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*mediafoundation_modulehandler_configuration_iterator).second.second->mute =
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second->mute =
    is_active;
#endif // ACE_WIN32 || ACE_WIN64
  GtkScale* scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_HSCALE_VOLUME_NAME)));
  ACE_ASSERT (scale_p);
  gtk_widget_set_sensitive (GTK_WIDGET (scale_p),
                            !is_active);
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

  bool is_active_b = gtk_toggle_button_get_active (toggleButton_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->displayAnalyzer =
        is_active_b;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->displayAnalyzer =
        is_active_b;
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  data_p->configuration->streamConfiguration.configuration_->displayAnalyzer =
    is_active_b;
#endif // ACE_WIN32 || ACE_WIN64

  GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_HBOX_VISUALIZATION_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            is_active_b);
} // togglebutton_visualization_toggled_cb

void
checkbutton_window_function_toggled_cb (GtkToggleButton* toggleButton_in,
                                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::checkbutton_window_function_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  bool is_active_b = gtk_toggle_button_get_active (toggleButton_in);

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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->applyWindowFunction =
        is_active_b;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->applyWindowFunction =
        is_active_b;
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
  (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->applyWindowFunction =
    is_active_b;
#endif // ACE_WIN32 || ACE_WIN64
} // checkbutton_window_function_toggled_cb

void
radiobutton_2d_toggled_cb (GtkToggleButton* toggleButton_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::radiobutton_2d_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  if (!gtk_toggle_button_get_active (toggleButton_in))
    return;
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkRadioButton* radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME)));
  ACE_ASSERT (radio_button_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
      (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode =
          (radio_button_p == GTK_RADIO_BUTTON (toggleButton_in) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE
                                                                : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
      (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode =
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
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
  (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode =
      (radio_button_p == GTK_RADIO_BUTTON (toggleButton_in) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE
                                                            : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM);
#endif // ACE_WIN32 || ACE_WIN64
} // radiobutton_2d_toggled_cb

void
radiobutton_noise_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::radiobutton_noise_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  if (!gtk_toggle_button_get_active (toggleButton_in))
    return;
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
  GtkRadioButton* radio_button_p =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_NOISE_NAME)));
  ACE_ASSERT (radio_button_p);
  GtkRadioButton* radio_button_2 =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_PINK_NAME)));
  ACE_ASSERT (radio_button_2);
#if defined (LIBNOISE_SUPPORT)
  GtkRadioButton* radio_button_3 =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_PERLIN_NAME)));
  ACE_ASSERT (radio_button_3);
#endif // LIBNOISE_SUPPORT
  GtkRadioButton* radio_button_4 =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SAWTOOTH_NAME)));
  ACE_ASSERT (radio_button_4);
  GtkRadioButton* radio_button_5 =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SINUS_NAME)));
  ACE_ASSERT (radio_button_5);
  GtkRadioButton* radio_button_6 =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SQUARE_NAME)));
  ACE_ASSERT (radio_button_6);
  //GtkRadioButton* radio_button_7 =
  //  GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_TRIANGLE_NAME)));
  //ACE_ASSERT (radio_button_7);

  enum Stream_MediaFramework_SoundGeneratorType noise_type_e =
    ((GTK_RADIO_BUTTON (toggleButton_in) == radio_button_p) ? STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_NOISE
  :  (GTK_RADIO_BUTTON (toggleButton_in) == radio_button_2) ? STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PINK_NOISE
#if defined (LIBNOISE_SUPPORT)
  :  (GTK_RADIO_BUTTON (toggleButton_in) == radio_button_3) ? STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PERLIN_NOISE
#endif // LIBNOISE_SUPPORT
  :  (GTK_RADIO_BUTTON (toggleButton_in) == radio_button_4) ? STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SAWTOOTH
  :  (GTK_RADIO_BUTTON (toggleButton_in) == radio_button_5) ? STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE
  :  (GTK_RADIO_BUTTON (toggleButton_in) == radio_button_6) ? STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SQUARE
  :  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_TRIANGLE);

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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      (*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration->type =
        noise_type_e;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      (*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration->type =
        noise_type_e;
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second->generatorConfiguration->type =
    noise_type_e;
#endif // ACE_WIN32 || ACE_WIN64

  // step1: load configuration options
  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_VBOX_NOISE_NAME)));
  ACE_ASSERT (box_p);
  GtkWidget* box_2 =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GRID_NOISE_NAME)));
  ACE_ASSERT (box_2);
  gtk_container_remove (GTK_CONTAINER (box_p),
                        box_2);
  GList* list_p = NULL, * list_2 = NULL;
  list_p = gtk_container_get_children (GTK_CONTAINER (box_p));
  for (list_2 = list_p; list_2; list_2 = g_list_next (list_2))
  {
    g_object_ref (G_OBJECT (GTK_WIDGET (list_2->data)));
    gtk_container_remove (GTK_CONTAINER (box_p),
                          GTK_WIDGET (list_2->data));
  } // end FOR
  g_list_free (list_p); list_p = NULL;
  gtk_container_add (GTK_CONTAINER (box_p),
                     box_2);
  GtkFrame* frame_p = NULL;
  switch (noise_type_e)
  {
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SAWTOOTH:
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE:
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SQUARE:
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_TRIANGLE:
    {
      frame_p =
        GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_SINUS_NAME)));
      ACE_ASSERT (frame_p);
      gtk_widget_unparent (GTK_WIDGET (frame_p));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_NOISE:
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PINK_NOISE:
      break;
#if defined (LIBNOISE_SUPPORT)
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PERLIN_NOISE:
    {
      frame_p =
        GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_PERLIN_NAME)));
      ACE_ASSERT (frame_p);
      gtk_widget_unparent (GTK_WIDGET (frame_p));
      break;
    }
#endif // LIBNOISE_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown noise type (was: %d), returning\n"),
                  noise_type_e));
      return;
    }
  } // end SWITCH
  if (frame_p)
    gtk_box_pack_start (box_p,
                        GTK_WIDGET (frame_p),
                        TRUE, // expand
                        TRUE, // fill
                        0);   // padding
} // radiobutton_noise_toggled_cb

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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzer3DMode =
        (is_active ? STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT
                   : STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzer3DMode =
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
#if defined (GTKGL_SUPPORT)
  (*modulehandler_configuration_iterator).second.second->spectrumAnalyzer3DMode =
      (is_active ? STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT
                 : STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID);
#endif // GTKGL_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  ui_cb_data_base_p->render3d = is_active;
} // togglebutton_3d_toggled_cb

// -----------------------------------------------------------------------------

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
button_device_settings_clicked_cb (GtkButton* button_in,
                                   gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_device_settings_clicked_cb"));

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
  ACE_ASSERT (iterator != state_r.builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (ui_cb_data_base_p);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      if (!(*directshow_modulehandler_configuration_iterator).second.second->builder)
        break; // not using DirectShow
      IBaseFilter* filter_p = NULL;
      HRESULT result =
        (*directshow_modulehandler_configuration_iterator).second.second->builder->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO_L,
                                                                                                     &filter_p);
      if (FAILED (result))
        break; // using Null renderer ?
      ACE_ASSERT (filter_p);
      ISpecifyPropertyPages* property_pages_p = NULL;
      result = filter_p->QueryInterface (IID_PPV_ARGS (&property_pages_p));
      ACE_ASSERT (SUCCEEDED (result) && property_pages_p);
      struct tagCAUUID uuids_a;
      ACE_OS::memset (&uuids_a, 0, sizeof (struct tagCAUUID));
      result = property_pages_p->GetPages (&uuids_a);
      ACE_ASSERT (SUCCEEDED (result) && uuids_a.pElems);
      property_pages_p->Release (); property_pages_p = NULL;
      IUnknown* iunknown_p = NULL;
      filter_p->QueryInterface (IID_PPV_ARGS (&iunknown_p));
      ACE_ASSERT (SUCCEEDED (result) && iunknown_p);
      //ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
      LCID locale_id = 1033;
      // display modal properties dialog
      // *TODO*: implement modeless support (i.e. return immediately)
      result =
        OleCreatePropertyFrame (NULL,//gdk_win32_window_get_impl_hwnd (window_p), // Parent window {NULL ? modeless : modal}
                                0, 0,                     // Reserved
#if defined (OLE2ANSI)
                                Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str (), // Caption for the dialog box
#else
                                ACE_TEXT_ALWAYS_WCHAR (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()), // Caption for the dialog box
#endif // OLE2ANSI
                                1,                        // Number of objects (just the filter)
                                &iunknown_p,              // Array of object pointers
                                uuids_a.cElems,           // Number of property pages
                                uuids_a.pElems,           // Array of property page CLSIDs
                                locale_id,                // Locale identifier
                                0, NULL);                 // Reserved
      ACE_ASSERT (SUCCEEDED (result));
      iunknown_p->Release (); iunknown_p = NULL;
      CoTaskMemFree (uuids_a.pElems); uuids_a.pElems = NULL;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (ui_cb_data_base_p);
      ACE_UNUSED_ARG (mediafoundation_ui_cb_data_p);
      //ACE_UNUSED_ARG (window_p);
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
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (ui_cb_data_base_p);
  ACE_UNUSED_ARG (ui_cb_data_p);
  //ACE_UNUSED_ARG (window_p);
#endif // ACE_WIN32 || ACE_WIN64
} // button_device_settings_clicked_cb

void
button_reset_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_reset_clicked_cb"));

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

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersConstIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());

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
        dynamic_cast<Test_U_AudioEffect_MediaFoundation_IStreamControl_t*> (mediafoundation_ui_cb_data_p->stream);
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
    stream_p->stop (false, // wait for completion ?
                    false, // recurse upstream ?
                    true); // high priority ?

  // wait for processing thread(s)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_base_p->UIState->lock);
    if (!ui_cb_data_base_p->progressData.pendingActions.empty ())
      return; // simply refuse to quit... :-)
    // *NOTE*: cannot wait on condition here, because it's signalled in
    //         idle_update_progress_cb(), which is never invoked while
    //         this thread is blocked...
      //ui_cb_data_base_p->UIState->condition.wait (NULL);
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

  // step2: destroy the dialog --> unrealize GTKGLArea --> free GL resources !
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  gtk_widget_destroy (GTK_WIDGET (dialog_p));

  // step3: initiate shutdown sequence (see idle_finalize_UI_cb() above)
  //        *NOTE*: the final gtk_main_quit() is called there
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  // step4: invoke signal handler
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
#endif // ACE_WIN32 || ACE_WIN64

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
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            0, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string effect_string = g_value_get_string (&value);
  g_value_unset (&value);

  // step1: load configuration options
  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_EFFECT_NAME)));
  ACE_ASSERT (box_p);
  GtkWidget* box_2 =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_EFFECT_2_NAME)));
  ACE_ASSERT (box_2);
  gtk_container_remove (GTK_CONTAINER (box_p),
                        box_2);
  GList* list_p = NULL, * list_2 = NULL;
  list_p = gtk_container_get_children (GTK_CONTAINER (box_p));
  for (list_2 = list_p; list_2; list_2 = g_list_next (list_2))
  {
    g_object_ref (G_OBJECT (GTK_WIDGET (list_2->data)));
    gtk_container_remove (GTK_CONTAINER (box_p),
                          GTK_WIDGET (list_2->data));
  } // end FOR
  g_list_free (list_p); list_p = NULL;
  gtk_container_add (GTK_CONTAINER (box_p),
                     box_2);
  GtkFrame* frame_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if GTK_CHECK_VERSION (2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  struct _GUID effect_GUID =
    Common_OS_Tools::StringToGUID (g_value_get_string (&value_2));
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
      (*directshow_modulehandler_configuration_iterator).second.second->effect =
        effect_GUID;
      if (InlineIsEqualGUID (effect_GUID, CLSID_CWMAudioAEC))
      {
        struct _DSCFXAec effect_options;
        effect_options.fEnable = TRUE;
        effect_options.fNoiseFill = FALSE;
        effect_options.dwMode = DSCFX_AEC_MODE_FULL_DUPLEX;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.AECOptions =
          effect_options;
      } // end IF
      //////////////////////////////////////
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_CHORUS))
      {
        struct _DSFXChorus effect_options;
        effect_options.fDelay = 16.0F;
        effect_options.fDepth = 10.0F;
        effect_options.fFeedback = 25.0F;
        effect_options.fFrequency = 1.1F;
        effect_options.fWetDryMix = 50.0F;
        effect_options.lPhase = DSFXCHORUS_PHASE_90;
        effect_options.lWaveform = DSFXCHORUS_WAVE_SIN;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.chorusOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_COMPRESSOR))
      {
        struct _DSFXCompressor effect_options;
        effect_options.fAttack = 10.0F;
        effect_options.fGain = 0.0F;
        effect_options.fPredelay = 4.0F;
        effect_options.fRatio = 3.0F;
        effect_options.fRelease = 200.0F;
        effect_options.fThreshold = -20.0F;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.compressorOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_DISTORTION))
      {
        struct _DSFXDistortion effect_options;
        effect_options.fEdge = 15.0F;
        effect_options.fGain = -18.0F;
        effect_options.fPostEQBandwidth = 2400.0F;
        effect_options.fPostEQCenterFrequency = 2400.0F;
        effect_options.fPreLowpassCutoff = 8000.0F;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.distortionOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_ECHO))
      {
        struct _DSFXEcho effect_options;
        effect_options.fFeedback = 50.0F;
        effect_options.fLeftDelay = 10.0F;
        effect_options.fRightDelay = 10.0F;
        effect_options.fWetDryMix = 50.0F;
        effect_options.lPanDelay = 0;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.echoOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_PARAMEQ))
      {
        struct _DSFXParamEq effect_options;
        effect_options.fBandwidth = 12.0F;
        effect_options.fCenter = 8000.0F;
        effect_options.fGain = 0.0F;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.equalizerOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_FLANGER))
      {
        struct _DSFXFlanger effect_options;
        effect_options.fDelay = 2.0F;
        effect_options.fDepth = 100.0F;
        effect_options.fFeedback = -50.0F;
        effect_options.fFrequency = 0.25F;
        effect_options.fWetDryMix = 50.0F;
        effect_options.lPhase = DSFXFLANGER_PHASE_ZERO;
        effect_options.lWaveform = DSFXFLANGER_WAVE_SIN;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.flangerOptions =
          effect_options;

        // *TODO*: reset option widgets as well

        frame_p =
          GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_EFFECT_WIN32_DS_FLANGER_OPTIONS_NAME)));
        ACE_ASSERT (frame_p);
        gtk_widget_unparent (GTK_WIDGET (frame_p));
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_GARGLE))
      {
        struct _DSFXGargle effect_options;
        effect_options.dwRateHz = 20;
        effect_options.dwWaveShape = DSFXGARGLE_WAVE_TRIANGLE;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.gargleOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_I3DL2REVERB))
      {
        struct _DSFXI3DL2Reverb effect_options;
        effect_options.flDecayHFRatio = DSFX_I3DL2REVERB_DECAYHFRATIO_DEFAULT;
        effect_options.flDecayTime = DSFX_I3DL2REVERB_DECAYTIME_DEFAULT;
        effect_options.flDensity = DSFX_I3DL2REVERB_DENSITY_DEFAULT;
        effect_options.flDiffusion = DSFX_I3DL2REVERB_DIFFUSION_DEFAULT;
        effect_options.flHFReference = DSFX_I3DL2REVERB_HFREFERENCE_DEFAULT;
        effect_options.flReflectionsDelay = DSFX_I3DL2REVERB_REFLECTIONSDELAY_DEFAULT;
        effect_options.flReverbDelay = DSFX_I3DL2REVERB_REVERBDELAY_DEFAULT;
        effect_options.flRoomRolloffFactor = DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_DEFAULT;
        effect_options.lReflections = DSFX_I3DL2REVERB_REFLECTIONS_DEFAULT;
        effect_options.lReverb = DSFX_I3DL2REVERB_REVERB_DEFAULT;
        effect_options.lRoom = DSFX_I3DL2REVERB_ROOM_DEFAULT;
        effect_options.lRoomHF = DSFX_I3DL2REVERB_ROOMHF_DEFAULT;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.reverbOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_WAVES_REVERB))
      {
        struct _DSFXWavesReverb effect_options;
        effect_options.fHighFreqRTRatio = DSFX_WAVESREVERB_HIGHFREQRTRATIO_DEFAULT;
        effect_options.fInGain = DSFX_WAVESREVERB_INGAIN_DEFAULT;
        effect_options.fReverbMix = DSFX_WAVESREVERB_REVERBMIX_DEFAULT;
        effect_options.fReverbTime = DSFX_WAVESREVERB_REVERBTIME_DEFAULT;
        (*directshow_modulehandler_configuration_iterator).second.second->effectOptions.wavesReverbOptions =
          effect_options;
      } // end ELSE IF
      //////////////////////////////////////
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown effect (was: \"%s\"; GUID: %s), continuing\n"),
                    ACE_TEXT (effect_string.c_str ()),
                    ACE_TEXT (Common_OS_Tools::GUIDToString (effect_GUID).c_str ())));
      } // end ELSE
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second->effect =
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
  (*modulehandler_configuration_iterator).second.second->effect = effect_string;
  (*modulehandler_configuration_iterator).second.second->effectOptions.clear ();
  if (effect_string == ACE_TEXT_ALWAYS_CHAR ("chorus"))
  {
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.5"));  // gain in
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.9"));  // gain out
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("50"));   // delay (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.4"));  // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.25")); // speed (Hz)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("2"));    // depth (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-t"));   // modulation
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("60"));   // delay (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.32")); // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.4"));  // speed (Hz)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("2.3"));  // depth (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-t"));   // modulation
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("40"));   // delay (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.3"));  // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.3"));  // speed (Hz)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("1.3"));  // depth (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-s"));   // modulation
  } // end IF
  else if (effect_string == ACE_TEXT_ALWAYS_CHAR ("echo"))
  {
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.8"));  // gain in
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.9"));  // gain out
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("100"));  // delay (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.3"));  // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("200"));  // delay (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.25")); // decay (% gain in)
  } // end ELSE IF
  else if (effect_string == ACE_TEXT_ALWAYS_CHAR ("phaser"))
  {
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.9"));  // gain in
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.85")); // gain out
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("4"));    // delay (ms)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.23")); // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("1.3"));  // speed
    (*modulehandler_configuration_iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-s"));   // modulation (sinusoidial)
  } // end ELSE IF
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("invalid/unknown effect (was: \"%s\"), using default options, continuing\n"),
                ACE_TEXT (effect_string.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  if (frame_p)
    gtk_box_pack_start (box_p, GTK_WIDGET (frame_p),
                        TRUE, // expand
                        TRUE, // fill
                        0);   // padding
} // combobox_effect_changed_cb

void
combobox_source_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_source_changed_cb"));

  // sanity check(s)
  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // <-- nothing selected
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
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  enum Test_U_AudioEffect_SourceType source_type_e =
    static_cast<enum Test_U_AudioEffect_SourceType> (g_value_get_uint (&value));
  g_value_unset (&value);
  enum Stream_MediaFramework_SoundGeneratorType noise_type_e =
    STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType =
        source_type_e;
      noise_type_e =
        (*directshow_modulehandler_configuration_iterator).second.second->generatorConfiguration->type;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration);

      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType =
        source_type_e;
      noise_type_e =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->generatorConfiguration->type;
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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->generatorConfiguration);

  ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType =
    source_type_e;
  noise_type_e =
    (*modulehandler_configuration_iterator).second.second->generatorConfiguration->type;
#endif // ACE_WIN32 || ACE_WIN64

  bool result_2 = false;
  GtkComboBox* combo_box_p = NULL;
  gint n_rows = 0;

  // step1: load configuration options
  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_VBOX_CONFIGURATION_NAME)));
  ACE_ASSERT (box_p);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_container_remove (GTK_CONTAINER (box_p),
                        GTK_WIDGET (combo_box_p));
  GList* list_p = NULL, *list_2 = NULL;
  list_p = gtk_container_get_children (GTK_CONTAINER (box_p));
  for (list_2 = list_p; list_2; list_2 = g_list_next (list_2))
  {
    g_object_ref (G_OBJECT (GTK_WIDGET (list_2->data)));
    gtk_container_remove (GTK_CONTAINER (box_p),
                          GTK_WIDGET (list_2->data));
  } // end FOR
  g_list_free (list_p); list_p = NULL;
  gtk_container_add (GTK_CONTAINER (box_p),
                     GTK_WIDGET (combo_box_p));
  GtkFrame* frame_p = NULL;
  bool load_formats_b = false;
  switch (source_type_e)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    {
      GtkBox* box_2 =
        GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_VBOX_FORMAT_OPTIONS_NAME)));
      ACE_ASSERT (box_2);
      gtk_widget_unparent (GTK_WIDGET (box_2));
      gtk_box_pack_start (box_p,
                          GTK_WIDGET (box_2),
                          TRUE, // expand
                          TRUE, // fill
                          0);   // padding
      frame_p =
        GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_DEVICE_NAME)));
      ACE_ASSERT (frame_p);
      gtk_widget_unparent (GTK_WIDGET (frame_p));
      break;
    }
    case AUDIOEFFECT_SOURCE_NOISE:
    {
      GtkBox* box_2 =
        GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_VBOX_FORMAT_OPTIONS_NAME)));
      ACE_ASSERT (box_2);
      gtk_widget_unparent (GTK_WIDGET (box_2));
      gtk_box_pack_start (box_p,
                          GTK_WIDGET (box_2),
                          TRUE, // expand
                          TRUE, // fill
                          0);   // padding
      frame_p =
        GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_NOISE_NAME)));
      ACE_ASSERT (frame_p);
      gtk_widget_unparent (GTK_WIDGET (frame_p));
      load_formats_b = true;
      break;
    }
    case AUDIOEFFECT_SOURCE_FILE:
    {
      frame_p =
        GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_FILE_NAME)));
      ACE_ASSERT (frame_p);
      gtk_widget_unparent (GTK_WIDGET (frame_p));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown source type (was: %d), returning\n"),
                  source_type_e));
      return;
    }
  } // end SWITCH
  gtk_box_pack_start (box_p,
                      GTK_WIDGET (frame_p),
                      TRUE, // expand
                      TRUE, // fill
                      0);   // padding

  if (!load_formats_b)
    goto continue_;

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  result_2 = load_all_formats (list_store_p);
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_formats(), returning\n")));
    return;
  } // end IF
  n_rows = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
    gint index_i = 0;
#if GTK_CHECK_VERSION (2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    index_i = 1;
    struct _GUID GUID_s = GUID_NULL;
    std::string format_string;
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        GUID_s =
          directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.subtype;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        HRESULT result =
          mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetGUID (MF_MT_SUBTYPE,
                                                                                                            &GUID_s);
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
    format_string = Common_OS_Tools::GUIDToString (GUID_s);
    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value,
                        format_string.c_str ());
#else
    index_i = 2;
    g_value_init (&value, G_TYPE_INT);
    g_value_set_int (&value,
                     ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format);
#endif // ACE_WIN32 || ACE_WIN64
    Common_UI_GTK_Tools::selectValue (combo_box_p,
                                      value,
                                      index_i);
    g_value_unset (&value);
  } // end IF

continue_:
  // activate source widget
  switch (source_type_e)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    {
      list_store_p =
        GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_DEVICE_NAME)));
      ACE_ASSERT (list_store_p);
      combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_DEVICE_NAME)));
      ACE_ASSERT (combo_box_p);
      n_rows =
        gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
      gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                                (n_rows > 0) && ui_cb_data_base_p->switchCaptureDevice);
      if (!n_rows)
        return;
      gint index_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      if (!(*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.empty ())
      {
        guint card_i =
          static_cast<guint> (Stream_MediaFramework_ALSA_Tools::getCardNumber ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier));
#if GTK_CHECK_VERSION(2,30,0)
        GValue value = G_VALUE_INIT;
#else
        GValue value;
        ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
        g_value_init (&value, G_TYPE_UINT);
        g_value_set_uint (&value, card_i);
        index_i =
          Common_UI_GTK_Tools::valueToIndex (gtk_combo_box_get_model (combo_box_p),
                                             value,
                                             2);
        if (index_i == -1)
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown device identifier (was: \"%s\"), continuing\n"),
                      ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ())));
          index_i = 0;
        } // end IF
      } // end IF
#endif // ACE_WIN32 || ACE_WIN64
      gtk_combo_box_set_active (combo_box_p, index_i);
      break;
    }
    case AUDIOEFFECT_SOURCE_NOISE:
    {
      GtkRadioButton* radio_button_p = NULL;
      switch (noise_type_e)
      {
        case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SAWTOOTH:
          radio_button_p =
            GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SAWTOOTH_NAME)));
          break;
        case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE:
          radio_button_p =
            GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SINUS_NAME)));
          break;
        case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SQUARE:
          radio_button_p =
            GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SQUARE_NAME)));
          break;
        case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_TRIANGLE:
          radio_button_p =
            GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_TRIANGLE_NAME)));
          break;
        case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_NOISE:
          radio_button_p =
            GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_NOISE_NAME)));
          break;
#if defined (LIBNOISE_SUPPORT)
        case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PERLIN_NOISE:
          radio_button_p =
            GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_PERLIN_NAME)));
          break;
#endif // LIBNOISE_SUPPORT
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown noise type (was: %d), returning\n"),
                      noise_type_e));
          return;
        }
      } // end SWITCH
      ACE_ASSERT (radio_button_p);
      g_signal_emit_by_name (G_OBJECT (radio_button_p),
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             userData_in);
      break;
    }
    case AUDIOEFFECT_SOURCE_FILE:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown source type (was: %d), returning\n"),
                  source_type_e));
      return;
    }
  } // end SWITCH

  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), TRUE);
} // combobox_source_changed_cb

void
combobox_device_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_device_changed_cb"));

  // sanity check(s)
  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // <-- nothing selected
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

  Stream_IStreamControlBase* stream_p = NULL;
#if GTK_CHECK_VERSION(2, 30, 0)
  GValue value = G_VALUE_INIT;
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value, value_2;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  std::string device_identifier_string;
  std::ostringstream converter;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::string format_string;
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      stream_p = directshow_ui_cb_data_p->stream;
      use_framework_source_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      stream_p = mediafoundation_ui_cb_data_p->stream;
      use_framework_source_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  stream_p = ui_cb_data_p->stream;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_DEVICE_NAME)));
  ACE_ASSERT (list_store_p);
  unsigned int card_id_i = std::numeric_limits<unsigned int>::max ();
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  device_identifier_string = g_value_get_string (&value);
  g_value_unset (&value);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  card_id_i = g_value_get_uint (&value_2);
  g_value_unset (&value_2);

  gint n_rows = 0;
  GtkToggleButton* toggle_button_p = NULL;
  GtkListStore* list_store_2 =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_2);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT && (_WIN32_WINNT >= 0x0602)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);

      switch (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
        {
          (*directshow_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          (*directshow_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._id =
            card_id_i;
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          (*directshow_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          (*directshow_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid =
            Common_OS_Tools::StringToGUID (device_identifier_string);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          goto error;
        }
      } // end SWITCH

      format_string =
        Common_OS_Tools::GUIDToString (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.subtype);

      if (directshow_ui_cb_data_p->streamConfiguration)
      {
        directshow_ui_cb_data_p->streamConfiguration->Release (); directshow_ui_cb_data_p->streamConfiguration = NULL;
      } // end IF
      if ((*directshow_modulehandler_configuration_iterator).second.second->builder)
      {
        Stream_MediaFramework_DirectShow_Tools::shutdown ((*directshow_modulehandler_configuration_iterator).second.second->builder);
        (*directshow_modulehandler_configuration_iterator).second.second->builder->Release (); (*directshow_modulehandler_configuration_iterator).second.second->builder = NULL;
      } // end IF
      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->filterGraphConfiguration.clear ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      (*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid =
        Common_OS_Tools::StringToGUID (device_identifier_string);

      struct _GUID GUID_2 = GUID_NULL;
      HRESULT result_3 =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetGUID (MF_MT_SUBTYPE,
                                                                                                          &GUID_2);
      if (FAILED (result_3))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_3).c_str ())));
        return;
      } // end IF
      format_string = Common_OS_Tools::GUIDToString (GUID_2);

      if ((*mediafoundation_modulehandler_configuration_iterator).second.second->session)
      {
        Stream_MediaFramework_MediaFoundation_Tools::shutdown ((*mediafoundation_modulehandler_configuration_iterator).second.second->session);
        (*mediafoundation_modulehandler_configuration_iterator).second.second->session->Release (); (*mediafoundation_modulehandler_configuration_iterator).second.second->session = NULL;
      } // end IF

      if (!use_framework_source_b)
        break;

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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Device_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier,
                                                                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getMediaSource(), returning\n")));
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
      goto error;
    }
  } // end SWITCH
#else
  ACE_UNUSED_ARG (card_id_i);
  if (ui_cb_data_base_p->switchCaptureDevice)
    (*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier = device_identifier_string;
#endif // ACE_WIN32 || ACE_WIN64

  bool result_2 = false;
  GtkScale* scale_p = NULL, *scale_2 = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_Dev_Mic_Source_DirectShow* directshow_source_impl_p = NULL;
  Test_U_AudioEffect_DirectShow_Source* directshow_source_impl_2 = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  IMFSampleGrabberSinkCallback2* sample_grabber_p = NULL;
#else
  IMFSampleGrabberSinkCallback* sample_grabber_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  IMFTopology* topology_p = NULL;
  HRESULT result = E_FAIL;
 
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Stream_Device_Identifier device_identifier;
      device_identifier.identifier._id = static_cast<int> (card_id_i);
      device_identifier.identifierDiscriminator = Stream_Device_Identifier::ID;
      IAMBufferNegotiation* buffer_negotiation_p = NULL;

      switch (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 = load_formats (card_id_i,
                                   list_store_2);
          goto continue_;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 =
            load_formats (Common_OS_Tools::StringToGUID (device_identifier_string),
                          list_store_2);
          goto continue_;
        }
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          goto error;
        }
      } // end SWITCH

      if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (device_identifier,
                                                            CLSID_AudioInputDeviceCategory,
                                                            (*directshow_modulehandler_configuration_iterator).second.second->builder,
                                                            buffer_negotiation_p,
                                                            directshow_ui_cb_data_p->streamConfiguration,
                                                            directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->filterGraphConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                    ACE_TEXT (device_identifier_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->builder);
      ACE_ASSERT (buffer_negotiation_p);
      ACE_ASSERT (directshow_ui_cb_data_p->streamConfiguration);
      buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

      result_2 =
        load_formats (directshow_ui_cb_data_p->streamConfiguration,
                      list_store_2);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      switch (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 = load_formats (card_id_i,
                                   list_store_2);
          goto continue_;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 =
            load_formats (Common_OS_Tools::StringToGUID (device_identifier_string),
                          list_store_2);
          goto continue_;
        }
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          goto error;
        }
      } // end SWITCH

      Common_IGetR_5_T<Test_U_Dev_Mic_Source_MediaFoundation>* iget_p =
          dynamic_cast<Common_IGetR_5_T<Test_U_Dev_Mic_Source_MediaFoundation>*> (stream_p);
      ACE_ASSERT (iget_p);
      Test_U_Dev_Mic_Source_MediaFoundation* writer_p =
          &const_cast<Test_U_Dev_Mic_Source_MediaFoundation&> (iget_p->getR_5 ());
      ACE_ASSERT (writer_p);
      sample_grabber_p = writer_p;

      struct _MFRatio pixel_aspect_ratio = { 1, 1 };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology ((*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier,
                                                                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                    media_source_p,
                                                                    sample_grabber_p,
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
      ACE_ASSERT (!(*mediafoundation_modulehandler_configuration_iterator).second.second->session);
      if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                     (*mediafoundation_modulehandler_configuration_iterator).second.second->session,
                                                                     true,  // is partial ?
                                                                     true)) // wait for completion ?
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n")));
        goto error;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      topology_p->Release (); topology_p = NULL;

      //if (!load_formats (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result_2 = load_formats (media_source_p,
                               list_store_2);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
continue_:
#else
  int result = -1;
  struct _snd_pcm_hw_params* format_p = NULL;
  long min_level_i = 0, max_level_i = 0, current_level_i = 0;

  if (ui_cb_data_p->handle)
  {
    result = snd_pcm_close (ui_cb_data_p->handle);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed ALSA device\n")));
    ACE_ASSERT (ui_cb_data_p->handle == (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->handle);
    ui_cb_data_p->handle = NULL;
    (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->handle =
        NULL;
  } // end IF
  ACE_ASSERT (!ui_cb_data_p->handle);
//  int mode = STREAM_LIB_ALSA_CAPTURE_DEFAULT_MODE;
  int mode = SND_PCM_NONBLOCK         |
             SND_PCM_NO_AUTO_RESAMPLE |
             SND_PCM_NO_AUTO_CHANNELS |
             SND_PCM_NO_SOFTVOL;
//   if ((*modulehandler_configuration_iterator).second.second->ALSAConfiguration->asynch)
//     mode |= SND_PCM_ASYNC;
  result =
    snd_pcm_open (&ui_cb_data_p->handle,
                  (*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str (),
                  SND_PCM_STREAM_CAPTURE,
                  mode);
  if ((result < 0) || !ui_cb_data_p->handle)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_open(\"%s\",%d) for capture: \"%s\", aborting\n"),
                ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ()),
                mode,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->handle =
    ui_cb_data_p->handle;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened ALSA device (capture) \"%s\"\n"),
              ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ())));

  result_2 =
      load_formats (ui_cb_data_p->handle,
                    (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->access,
                    list_store_2);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_formats(), returning\n")));
    goto error;
  } // end IF

  n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_2), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
    gint index_i = 0;
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value,
                        format_string.c_str ());
    index_i = 1;
#else
    g_value_init (&value, G_TYPE_INT);
    g_value_set_int (&value,
                     ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format);
    index_i = 2;
#endif // ACE_WIN32 || ACE_WIN64
    index_i =
        Common_UI_GTK_Tools::valueToIndex (gtk_combo_box_get_model (combo_box_p),
                                           value,
                                           index_i);
    if (index_i == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown format (was: \"%s\"), continuing\n"),
                  ACE_TEXT (format_string.c_str ())));
#else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown format (was: %d), continuing\n"),
                  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format));
#endif // ACE_WIN32 || ACE_WIN64
      index_i = 0;
    } // end IF
    g_value_unset (&value);
    gtk_combo_box_set_active (combo_box_p, index_i);
  } // end IF

  toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), TRUE);

  // get/set capture volume / boost levels
  scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_HSCALE_DEVICE_VOLUME_NAME)));
  ACE_ASSERT (scale_p);
  scale_2 =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_HSCALE_DEVICE_BOOST_NAME)));
  ACE_ASSERT (scale_2);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // retrieve volume control handle
  // step1: retrieve DirectSound device GUID from wave device id
  struct _GUID GUID_s =
    Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (card_id_i,
                                                                            true); // capture
  ACE_ASSERT (!InlineIsEqualGUID (GUID_s, GUID_NULL));
  IAudioEndpointVolume* i_audio_endpoint_volume_p =
    Stream_MediaFramework_DirectSound_Tools::getMasterVolumeControl (GUID_s);
  if (!i_audio_endpoint_volume_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getMasterVolumeControl(\"%s\") (waveIn card id was: %u), returning\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                card_id_i));
    goto error_2;
  } // end IF
  float volume_level_f = 0.0;
  result =
    i_audio_endpoint_volume_p->GetMasterVolumeLevelScalar (&volume_level_f);
  ACE_ASSERT (SUCCEEDED (result));

  float min_level_f = 0.0F, max_level_f = 0.0F, stepping_f = 0.0F, boost_f = 0.0F;
  IAudioVolumeLevel* i_audio_volume_level_p =
    Stream_MediaFramework_DirectSound_Tools::getMicrophoneBoostControl (GUID_s);
  if (!i_audio_volume_level_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getMicrophoneBoostControl(\"%s\") (waveIn card id was: %u), continuing\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                card_id_i));
    goto continue_2;
  } // end IF
  result = i_audio_volume_level_p->GetLevelRange (0,
                                                  &min_level_f,
                                                  &max_level_f,
                                                  &stepping_f);
  ACE_ASSERT (SUCCEEDED (result));
  gtk_range_set_range (GTK_RANGE (scale_2),
                       static_cast<gdouble> (min_level_f),
                       static_cast<gdouble> (max_level_f));
  gtk_range_set_increments (GTK_RANGE (scale_2),
                            static_cast<gdouble> (stepping_f),
                            static_cast<gdouble> (stepping_f));

  gtk_scale_clear_marks (GTK_SCALE (scale_2));
  converter.precision (2);
  converter << std::fixed; // for fixed point notation
  for (float i = min_level_f;
       i <= max_level_f;
       i += stepping_f)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << i;
    gtk_scale_add_mark (GTK_SCALE (scale_2),
                        static_cast<gdouble> (i),
                        GTK_POS_TOP,
                        converter.str ().c_str ());
  } // end FOR
  result = i_audio_volume_level_p->GetLevel (0, &boost_f);
  ACE_ASSERT (SUCCEEDED (result));

  goto continue_2;

error_2:
  if (i_audio_endpoint_volume_p)
  {
    i_audio_endpoint_volume_p->Release (); i_audio_endpoint_volume_p = NULL;
  } // end IF
  goto error;

continue_2:
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (directshow_ui_cb_data_p->captureVolumeControl)
      {
        directshow_ui_cb_data_p->captureVolumeControl->Release (); directshow_ui_cb_data_p->captureVolumeControl = NULL;
      } // end IF
      directshow_ui_cb_data_p->captureVolumeControl = i_audio_endpoint_volume_p;
      if (directshow_ui_cb_data_p->boostControl)
      {
        directshow_ui_cb_data_p->boostControl->Release (); directshow_ui_cb_data_p->boostControl = NULL;
      } // end IF
      directshow_ui_cb_data_p->boostControl = i_audio_volume_level_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (mediafoundation_ui_cb_data_p->captureVolumeControl)
      {
        mediafoundation_ui_cb_data_p->captureVolumeControl->Release (); mediafoundation_ui_cb_data_p->captureVolumeControl = NULL;
      } // end IF
      mediafoundation_ui_cb_data_p->captureVolumeControl = i_audio_endpoint_volume_p;
      if (mediafoundation_ui_cb_data_p->boostControl)
      {
        mediafoundation_ui_cb_data_p->boostControl->Release (); mediafoundation_ui_cb_data_p->boostControl = NULL;
      } // end IF
      mediafoundation_ui_cb_data_p->boostControl = i_audio_volume_level_p;

      if (media_source_p)
      {
        media_source_p->Release (); media_source_p = NULL;
      } // end IF
      if (sample_grabber_p)
      {
        sample_grabber_p->Release (); sample_grabber_p = NULL;
      } // end IF
      if (topology_p)
      {
        topology_p->Release (); topology_p = NULL;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      i_audio_endpoint_volume_p->Release ();
      i_audio_volume_level_p->Release ();
      goto error;
    }
  } // end SWITCH
  gtk_range_set_value (GTK_RANGE (scale_p),
                       static_cast<gdouble> (volume_level_f) * 100.0);
  gtk_range_set_value (GTK_RANGE (scale_2),
                       static_cast<gdouble> (boost_f));
#else
  if (!Stream_MediaFramework_ALSA_Tools::getVolumeLevels ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_VOLUME_NAME),
                                                          true, // capture
                                                          min_level_i,
                                                          max_level_i,
                                                          current_level_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getVolumeLevels(\"%s\",\"%s\"), continuing\n"),
                ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ()),
                ACE_TEXT (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_VOLUME_NAME)));
    goto continue_;
  } // end IF
  gtk_range_set_range (GTK_RANGE (scale_p),
                       static_cast<gdouble> (min_level_i),
                       static_cast<gdouble> (max_level_i));
  gtk_range_set_increments (GTK_RANGE (scale_p),
                            static_cast<gdouble> (1),
                            static_cast<gdouble> (1));
  gtk_range_set_value (GTK_RANGE (scale_p),
                       static_cast<gdouble> (current_level_i));

continue_:
  if (!Stream_MediaFramework_ALSA_Tools::getVolumeLevels ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME),
                                                          true, // capture
                                                          min_level_i,
                                                          max_level_i,
                                                          current_level_i))
  {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getVolumeLevels(\"%s\",\"%s\"), continuing\n"),
                 ACE_TEXT ((*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier.c_str ()),
                 ACE_TEXT (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME)));
      goto continue_2;
  } // end IF
  gtk_range_set_range (GTK_RANGE (scale_2),
                       static_cast<gdouble> (min_level_i),
                       static_cast<gdouble> (max_level_i));
  gtk_range_set_increments (GTK_RANGE (scale_2),
                            static_cast<gdouble> (1),
                            static_cast<gdouble> (1));
  gtk_scale_clear_marks (scale_2);
  for (long i = min_level_i;
       i <= max_level_i;
       i += 1)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << i;
    gtk_scale_add_mark (scale_2,
                        static_cast<gdouble> (i),
                        GTK_POS_TOP,
                        converter.str ().c_str ());
  } // end FOR
  gtk_range_set_value (GTK_RANGE (scale_2),
                       static_cast<gdouble> (current_level_i));
continue_2:
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
} // combobox_device_changed_cb

void
combobox_format_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  // sanity check(s)
  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // --> clear
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
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  bool load_all_formats_b = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      load_all_formats_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
      use_framework_source_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      load_all_formats_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
      use_framework_source_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION);
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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  load_all_formats_b =
    (ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64

#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
  struct _GUID GUID_s = Common_OS_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);

  bool result_2 = false;
  unsigned int sample_rate_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT && (_WIN32_WINNT >= 0x0602)
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_DEVICE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkListStore* list_store_2 =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_DEVICE_NAME)));
  ACE_ASSERT (list_store_2);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
  GValue value_3 = G_VALUE_INIT;
#else
  GValue value_2, value_3;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  ACE_OS::memset (&value_3, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_2),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_2),
                            &iterator_2,
                            2, &value_3);
  ACE_ASSERT (G_VALUE_TYPE (&value_3) == G_TYPE_UINT);
  struct _GUID GUID_2 =
    Common_OS_Tools::StringToGUID (g_value_get_string (&value_2));
  g_value_unset (&value_2);
  unsigned int index_i = g_value_get_uint (&value_3);
  g_value_unset (&value_3);
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      ACE_ASSERT (audio_info_header_p);

      Stream_MediaFramework_DirectShow_Tools::setFormat (GUID_s,
                                                         directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      sample_rate_i = audio_info_header_p->nSamplesPerSec;

      if (load_all_formats_b)
      {
        result_2 = load_all_sample_rates (list_store_p);
        break;
      } // end IF

      switch (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 = load_sample_rates (index_i,
                                        GUID_s,
                                        list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 = load_sample_rates (GUID_2,
                                        GUID_s,
                                        list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
        {
          result_2 =
            load_sample_rates (directshow_ui_cb_data_p->streamConfiguration,
                               GUID_s,
                               list_store_p);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
        }
      } // end SWITCH
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);

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
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                                                                            &sample_rate_i);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      if (load_all_formats_b)
      {
        result_2 = load_all_sample_rates (list_store_p);
        break;
      } // end IF

      switch (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 = load_sample_rates (index_i,
                                        GUID_s,
                                        list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 = load_sample_rates (GUID_2,
                                        GUID_s,
                                        list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
        { ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->session);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
          if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second->session,
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
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
        }
      } // end SWITCH
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

  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format =
    format_e;
  sample_rate_i =
    ui_cb_data_p->configuration->streamConfiguration.configuration_->format.rate;

  if (load_all_formats_b)
    result_2 = load_all_sample_rates (list_store_p);
  else
  { ACE_ASSERT (ui_cb_data_p->handle);
    result_2 =
      load_sample_rates (ui_cb_data_p->handle,
                         ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                         list_store_p);
  } // end ELSE
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
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
    g_value_init (&value, G_TYPE_UINT);
    g_value_set_uint (&value,
                      sample_rate_i);
    Common_UI_GTK_Tools::selectValue (combo_box_p,
                                      value,
                                      1);
    g_value_unset (&value);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
} // combobox_format_changed_cb

void
combobox_frequency_changed_cb (GtkWidget* widget_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_frequency_changed_cb"));

  // sanity check(s)
  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // --> clear
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  bool load_all_formats_b = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      load_all_formats_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
      use_framework_source_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      load_all_formats_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
      use_framework_source_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION);
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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  load_all_formats_b =
    (ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
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
  struct _GUID GUID_s = Common_OS_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
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
  unsigned int sample_bits_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  GtkComboBox* combo_box_2 =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_DEVICE_NAME)));
  ACE_ASSERT (combo_box_2);
  GtkListStore* list_store_2 =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_DEVICE_NAME)));
  ACE_ASSERT (list_store_2);
  if (!gtk_combo_box_get_active_iter (combo_box_2,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
  GValue value_3 = G_VALUE_INIT;
#else
  GValue value_2, value_3;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  ACE_OS::memset (&value_3, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_2),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_2),
                            &iterator_2,
                            2, &value_3);
  ACE_ASSERT (G_VALUE_TYPE (&value_3) == G_TYPE_UINT);
  struct _GUID GUID_2 =
    Common_OS_Tools::StringToGUID (g_value_get_string (&value_2));
  g_value_unset (&value_2);
  unsigned int index_i = g_value_get_uint (&value_3);
  g_value_unset (&value_3);
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      ACE_ASSERT (audio_info_header_p);

      audio_info_header_p->nSamplesPerSec = sample_rate;
      sample_bits_i = audio_info_header_p->wBitsPerSample;

      if (load_all_formats_b)
      {
        result_2 = load_all_sample_resolutions (list_store_p);
        break;
      } // end IF

      switch (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 =
            load_sample_resolutions (index_i,
                                     GUID_s,
                                     sample_rate,
                                     list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 =
            load_sample_resolutions (GUID_2,
                                     GUID_s,
                                     sample_rate,
                                     list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
        {
          result_2 =
            load_sample_resolutions (directshow_ui_cb_data_p->streamConfiguration,
                                     GUID_s,
                                     sample_rate,
                                     list_store_p);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
        }
      } // end SWITCH
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);

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
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                                            &sample_bits_i);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      if (load_all_formats_b)
      {
        result_2 = load_all_sample_resolutions (list_store_p);
        break;
      } // end IF

      switch (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 =
            load_sample_resolutions (index_i,
                                     GUID_s,
                                     sample_rate,
                                     list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 =
            load_sample_resolutions (GUID_2,
                                     GUID_s,
                                     sample_rate,
                                     list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
        {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
          ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->session);
          if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second->session,
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
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
        }
      } // end SWITCH
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

  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.rate =
    sample_rate;
  sample_bits_i =
    snd_pcm_format_width (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format);

  if (load_all_formats_b)
    result_2 = load_all_sample_resolutions (list_store_p);
  else
  { ACE_ASSERT (ui_cb_data_p->handle);
    result_2 =
      load_sample_resolutions (ui_cb_data_p->handle,
                               ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                               list_store_p);
  } // end ELSE
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
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
    g_value_init (&value, G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
#endif // ACE_WIN32 || ACE_WIN64
    g_value_set_uint (&value,
                      sample_bits_i);
    Common_UI_GTK_Tools::selectValue (combo_box_p,
                                      value,
                                      1);
    g_value_unset (&value);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // --> clear
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  bool load_all_formats_b = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      load_all_formats_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
      use_framework_source_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      load_all_formats_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
      use_framework_source_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION);
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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  load_all_formats_b =
    (ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType != AUDIOEFFECT_SOURCE_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
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
  struct _GUID GUID_s = Common_OS_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
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
  g_value_unset (&value);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME)));
  ACE_ASSERT (list_store_p);

  bool result_2 = false;
  unsigned int channels_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  GtkComboBox* combo_box_2 =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_DEVICE_NAME)));
  ACE_ASSERT (combo_box_2);
  GtkListStore* list_store_2 =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_DEVICE_NAME)));
  ACE_ASSERT (list_store_2);
  if (!gtk_combo_box_get_active_iter (combo_box_2,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
  GValue value_3 = G_VALUE_INIT;
#else
  GValue value_2, value_3;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  ACE_OS::memset (&value_3, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_2),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_2),
                            &iterator_2,
                            2, &value_3);
  ACE_ASSERT (G_VALUE_TYPE (&value_3) == G_TYPE_UINT);
  struct _GUID GUID_2 =
    Common_OS_Tools::StringToGUID (g_value_get_string (&value_2));
  g_value_unset (&value_2);
  unsigned int index_i = g_value_get_uint (&value_3);
  g_value_unset (&value_3);
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      ACE_ASSERT (audio_info_header_p);

      audio_info_header_p->wBitsPerSample = bits_per_sample;
      channels_i = audio_info_header_p->nChannels;

      if (load_all_formats_b)
      {
        result_2 = load_all_channels (list_store_p);
        break;
      } // end IF

      switch (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 = load_channels (index_i,
                                    GUID_s,
                                    sample_rate,
                                    bits_per_sample,
                                    list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 = load_channels (GUID_2,
                                    GUID_s,
                                    sample_rate,
                                    bits_per_sample,
                                    list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
        {
          result_2 =
            load_channels (directshow_ui_cb_data_p->streamConfiguration,
                           GUID_s,
                           sample_rate,
                           bits_per_sample,
                           list_store_p);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
        }
      } // end SWITCH
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);

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
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                            &channels_i);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      if (load_all_formats_b)
      {
        result_2 = load_all_channels (list_store_p);
        break;
      } // end IF

      switch (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          result_2 = load_channels (index_i,
                                    GUID_s,
                                    sample_rate,
                                    bits_per_sample,
                                    list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          result_2 = load_channels (GUID_2,
                                    GUID_s,
                                    sample_rate,
                                    bits_per_sample,
                                    list_store_p);
          break;
        }
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
        {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
          ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->session);
          if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second->session,
                                                                            media_source_p))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), returning\n")));
            return;
          } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
          ACE_ASSERT (media_source_p);

          //if (!load_channels (data_p->configuration->moduleHandlerConfiguration.sourceReader,
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
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
        }
      } // end SWITCH
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
  ACE_UNUSED_ARG (format_e);
  ACE_UNUSED_ARG (bits_per_sample);
  channels_i =
    ui_cb_data_p->configuration->streamConfiguration.configuration_->format.channels;

  if (load_all_formats_b)
    result_2 = load_all_channels (list_store_p);
  else
  { ACE_ASSERT (ui_cb_data_p->handle);
    result_2 =
      load_channels (ui_cb_data_p->handle,
                     ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                     list_store_p);
  } // end ELSE
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
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
    g_value_init (&value, G_TYPE_UINT);
    g_value_set_uint (&value,
                      channels_i);
    Common_UI_GTK_Tools::selectValue (combo_box_p,
                                      value,
                                      1);
    g_value_unset (&value);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // --> clear
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
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      use_framework_source_b =
        (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      use_framework_source_b =
        (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION);
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
  struct _GUID GUID_s = Common_OS_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
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
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
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
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      ACE_ASSERT (audio_info_header_p);

      audio_info_header_p->nChannels = number_of_channels;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);

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
  ACE_ASSERT (ui_cb_data_p->configuration);

  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.channels =
      number_of_channels;
#endif // ACE_WIN32 || ACE_WIN64

  update_media_type (userData_in);
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
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStream_t* istream_p = NULL;
  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode =
      STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID;
  unsigned int sample_size = 0; // bytes
  bool is_signed_format = true;
  bool is_float_format = false;
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      if (!directshow_ui_cb_data_p->stream->isRunning ())
        return FALSE;
      istream_p = dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      mode =
        (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->outputFormat.cbFormat == sizeof (struct tWAVEFORMATEX));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> ((*directshow_modulehandler_configuration_iterator).second.second->outputFormat.pbFormat);
      ACE_ASSERT (waveformatex_p);
      sample_size = waveformatex_p->wBitsPerSample / 8;
      channels = waveformatex_p->nChannels;
      // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
      //         are unsigned values. (Each audio sample has the range 0255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      is_signed_format = !(sample_size == 1);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      if (!mediafoundation_ui_cb_data_p->stream->isRunning ())
        return FALSE;
      istream_p = dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
      mode =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                                        &sample_size);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return FALSE;
      } // end IF
      sample_size /= 8;
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                        &channels);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return FALSE;
      } // end IF
      // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
      //         are unsigned values. (Each audio sample has the range 0255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      is_signed_format = !(sample_size == 1);
      is_float_format =
        Stream_MediaFramework_MediaFoundation_Tools::isFloat ((*mediafoundation_modulehandler_configuration_iterator).second.second->outputFormat);
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
  if (!ui_cb_data_p->stream->isRunning ())
    return FALSE;
  istream_p = dynamic_cast<Stream_IStream_t*> (ui_cb_data_p->stream);
  mode =
    (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
  is_signed_format =
      snd_pcm_format_signed (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format);
  sample_size =
      (snd_pcm_format_width (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format) / 8);
  channels =
    ui_cb_data_p->configuration->streamConfiguration.configuration_->format.channels;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (istream_p);

  const Stream_Module_t* module_p = NULL;
  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
    return FALSE;
  } // end IF
  Common_Math_FFT_Float_t* math_fft_p =
    dynamic_cast<Common_Math_FFT_Float_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (unlikely (!math_fft_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_Math_FFT_T<float>*>(%@), returning\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    return FALSE;
  } // end IF

  GtkAllocation allocation;
  gtk_widget_get_allocation (widget_in,
                             &allocation);
  double half_height = allocation.height / 2.0;
  uint64_t maximum_value =
    (is_float_format ? 1 : Common_Tools::max<uint64_t> (sample_size,
                                                        is_signed_format));
  std::ostringstream converter;
  switch (mode)
  {
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE:
    {
      // *TODO*: the value type depends on the format, so this isn't accurate
      if (is_signed_format)
        converter <<
          static_cast<int64_t> (((half_height - y_in) * static_cast<int64_t> (maximum_value)) / half_height);
      else
        converter <<
          static_cast<uint64_t> (((half_height - y_in) * maximum_value) / half_height);
      break;
    }
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM:
    {
      // *TODO*: the value type depends on the format, so this isn't accurate
      if (is_float_format)
        converter <<
          (static_cast<float> (allocation.height - y_in) * maximum_value) / static_cast<float> (allocation.height);
      else if (is_signed_format)
        converter <<
          static_cast<int64_t> (((half_height - y_in) * static_cast<int64_t> (maximum_value)) / half_height);
      else
        converter <<
          static_cast<uint64_t> (((half_height - y_in) * maximum_value) / half_height);
      unsigned int allocation_per_channel = (allocation.width / channels);
      unsigned int slot =
        static_cast<unsigned int> ((x_in % allocation_per_channel) * (math_fft_p->Slots () / static_cast<double> (allocation_per_channel)));
      converter << ACE_TEXT_ALWAYS_CHAR (", ")
                << math_fft_p->Frequency (slot)
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
////  GValue value = ;
////#else
////  GValue value;
////  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
////  g_value_init (&value, G_TYPE_BOOLEAN);
////#endif // GTK_CG_VALUE_INITHECK_VERSION (2,30,0)
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

  Test_U_Common_ISet_t* notification_p = NULL;
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      //ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      //directshow_modulehandler_configuration_iterator =
      //  directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      //ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      notification_p = directshow_ui_cb_data_p->resizeNotification;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      //ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      //mediafoundation_modulehandler_configuration_iterator =
      //  mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      //ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  //ACE_ASSERT (ui_cb_data_p->configuration);
  //Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T streamconfiguration_iterator =
  //    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  //ACE_ASSERT (streamconfiguration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  notification_p = ui_cb_data_p->resizeNotification;
#endif // ACE_WIN32 || ACE_WIN64

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  if (!window_p)
    return; // <-- not realized yet

  if (likely (notification_p))
  {
    try {
      notification_p->setP (window_p);
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  if (widget_in != GTK_WIDGET (drawing_area_p))
    return TRUE;
  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  ACE_ASSERT (window_p);

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
  notification_p = ui_cb_data_p->resizeNotification;
#endif // ACE_WIN32 || ACE_WIN64
  if (notification_p)
  {
    try {
      notification_p->setP (window_p);
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
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  // sanity check(s)
  ui_cb_data_base_p->spectrumAnalyzerCBData.window =
    gtk_widget_get_window (widget_in);
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.window)
    return FALSE; // not realized yet
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.dispatch)
    return FALSE; // stream not running (yet)
  ui_cb_data_base_p->spectrumAnalyzerCBData.context = context_in;

  try {
    ui_cb_data_base_p->spectrumAnalyzerCBData.dispatch->dispatch (&ui_cb_data_base_p->spectrumAnalyzerCBData);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  return FALSE;
}
#else
gboolean
drawingarea_expose_event_cb (GtkWidget* widget_in,
                             GdkEvent* event_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_expose_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  // sanity check(s)
  ui_cb_data_base_p->spectrumAnalyzerCBData.window =
    gtk_widget_get_window (widget_in);
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.window)
    return FALSE; // not realized yet
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.dispatch)
    return FALSE; // stream not running (yet)
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.context)
  {
    ui_cb_data_base_p->spectrumAnalyzerCBData.context =
      gdk_cairo_create (GDK_DRAWABLE (ui_cb_data_base_p->spectrumAnalyzerCBData.window));
    if (unlikely (!ui_cb_data_base_p->spectrumAnalyzerCBData.context))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
      return FALSE;
    } // end IF
  } // end IF

  try {
    ui_cb_data_base_p->spectrumAnalyzerCBData.dispatch->dispatch (&ui_cb_data_base_p->spectrumAnalyzerCBData);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

//  cairo_destroy (context_p); context_p = NULL;

  return FALSE;
} // drawingarea_expose_event_cb
#endif // GTK_CHECK_VERSION(3,0,0)

void
filechooserbutton_file_file_set_cb (GtkFileChooserButton* button_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_file_file_set_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
//  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != state_r.builders.end ());

  char* filename_p = NULL;
  GFile* file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (button_in));
  if (!file_p)
    return;
  filename_p = g_file_get_path (file_p);
  if (!filename_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));
    g_object_unref (file_p); file_p = NULL;
    return;
  } // end IF
  g_object_unref (file_p); file_p = NULL;
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*directshow_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
        Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*mediafoundation_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
        Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
    Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
#endif // ACE_WIN32 || ACE_WIN64
  g_free (filename_p);
} // filechooserbutton_file_file_set_cb

void
filechooserbutton_save_file_set_cb (GtkFileChooserButton* button_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_save_file_set_cb"));

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
  ACE_ASSERT (iterator != state_r.builders.end ());

  char* filename_p = NULL;
  GFile* file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (button_in));
  if (!file_p)
    return;
  filename_p = g_file_get_path (file_p);
  if (!filename_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));
    g_object_unref (file_p); file_p = NULL;
    return;
  } // end IF
  g_object_unref (file_p); file_p = NULL;
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
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*directshow_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
        Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*mediafoundation_modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
        Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
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
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
    Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
#endif // ACE_WIN32 || ACE_WIN64
  g_free (filename_p);
} // filechooserbutton_save_file_set_cb

//void
//filechooserdialog_response_cb (GtkDialog* dialog_in,
//                               int responseId_in,
//                               gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::filechooserdialog_response_cb"));
//
//  // sanity check(s)
//  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
//    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
//  ACE_ASSERT (ui_cb_data_base_p);
//
//  Common_UI_GTK_Manager_t* gtk_manager_p =
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//  ACE_ASSERT (gtk_manager_p);
//  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
//
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != state_r.builders.end ());
//
//  GtkFileChooserButton* file_chooser_button_p =
//    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
//  ACE_ASSERT (file_chooser_button_p);
//
//  switch (responseId_in)
//  {
//    case GTK_RESPONSE_OK:
//    {
//      filechooserbutton_destination_file_set_cb (file_chooser_button_p,
//                                                 userData_in);
//      break;
//    }
//    case GTK_RESPONSE_DELETE_EVENT: // ESC
//    case GTK_RESPONSE_CANCEL:
//    default:
//      break;
//  } // end SWITCH
//}
//
//void
//filechooser_file_activated_cb (GtkFileChooser* chooser_in,
//                               gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::filechooser_file_activated_cb"));
//
//  ACE_UNUSED_ARG (userData_in);
//
//  //ACE_ASSERT (false); // *TODO*
//} // filechooser_file_activated_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
