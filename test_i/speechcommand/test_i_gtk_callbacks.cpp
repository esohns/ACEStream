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

#include "test_i_gtk_callbacks.h"

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "functiondiscoverykeys_devpkey.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "gdk/gdkkeysyms.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#else
#include "gdk/gdkx.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "test_i_speechcommand_defines.h"
#include "test_i_stream.h"

#if defined (GTKGL_SUPPORT)
#include "test_i_gl_callbacks.h"
#endif // GTKGL_SUPPORT

// global variables
bool untoggling_record_button = false;

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_capture_devices (GtkListStore* listStore_in,
                      enum Stream_Device_Capturer capturer_in)
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
          Common_Tools::GUIDToString (Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (i,
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
                                                   DEVICE_STATEMASK_ALL,
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
        GUID_s = Common_Tools::StringToGUID (device_identifier_string);
        device_id_i =
          Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (GUID_s);
        if (unlikely (device_id_i == std::numeric_limits<unsigned int>::max ()))
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId(%s), continuing\n"),
                      ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
          continue;
        } // end IF

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
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_NAME_STRING,
                              &variant_s,
                              0);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_NAME_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
          goto error;
        } // end IF
        device_friendlyname_string =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant_s.bstrVal));
        VariantClear (&variant_s);
        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING,
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
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));

        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_PATH_STRING,
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
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_PATH_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));

        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_ID_STRING,
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
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_ID_STRING),
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
      if (FAILED (result_2) || !attributes_p)
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
      if (FAILED (result_2) || !devices_pp)
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
                    ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", continuing\n")));
        continue;
      } // end IF
      if ((ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("default")) == 0)             ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("dmix:"), 5) == 0)           ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("dsnoop:"), 7) == 0)         ||
  //        (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("hw:CARD=MID,DEV=0")) == 0)   ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("front:"), 6) == 0)          ||
          (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("null")) == 0)                ||
          (ACE_OS::strncmp (string_2, ACE_TEXT_ALWAYS_CHAR ("plughw:"), 7) == 0)         ||
          (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("pipewire")) == 0)            ||
          (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("pulse")) == 0)               ||
  //        (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("sysdefault:CARD=MID")) == 0) ||
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
      std::string::size_type position_i = device_name_string.find (':', 0);
      if (position_i != std::string::npos)
        device_name_string.erase (position_i, std::string::npos);
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

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stream processing thread (id: %t) starting\n")));

  // initialize return value(s)
  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
  bool COM_initialized = Common_Tools::initializeCOM ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  struct Test_I_UI_ThreadData* thread_data_base_p =
    static_cast<struct Test_I_UI_ThreadData*> (arg_in);
  ACE_ASSERT (thread_data_base_p);

  Common_UI_GTK_BuildersConstIterator_t iterator;
  Common_UI_GTK_State_t* state_p = NULL;
  std::ostringstream converter;
  Test_I_SessionData* session_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  const Test_I_SpeechCommand_DirectShow_SessionData_t* directshow_session_data_container_p =
    NULL;
  Test_I_SpeechCommand_DirectShow_SessionData* directshow_session_data_p =
    NULL;
  const Test_I_SpeechCommand_MediaFoundation_SessionData_t* mediafoundation_session_data_container_p =
    NULL;
  Test_I_SpeechCommand_MediaFoundation_SessionData* mediafoundation_session_data_p =
    NULL;
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      struct Test_I_DirectShow_UI_ThreadData* thread_data_p =
        static_cast<struct Test_I_DirectShow_UI_ThreadData*> (arg_in);
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
      struct Test_I_MediaFoundation_UI_ThreadData* thread_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_ThreadData*> (arg_in);
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
  struct Test_I_ALSA_UI_ThreadData* thread_data_p = NULL;
  const Test_I_SpeechCommand_ALSA_SessionData_t* session_data_container_p =
    NULL;

  // sanity check(s)
  thread_data_p =
    static_cast<struct Test_I_ALSA_UI_ThreadData*> (arg_in);
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p = thread_data_p->CBData;
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  state_p = ui_cb_data_p->UIState;
  ACE_ASSERT (state_p);
  iterator =
    state_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_p->builders.end ());
#endif // ACE_WIN32 || ACE_WIN64

  // generate context id
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
  GtkStatusbar* statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)

  bool result_2 = false;
  Stream_IStream_t* istream_p = NULL;
  Stream_IStreamControlBase* istream_control_p = NULL;
  const Stream_Module_t* module_p = NULL;
  Common_IDispatch* dispatch_p = NULL;
  Test_I_Common_ISet_t* resize_notification_p = NULL;
  guint event_source_id = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Test_I_DirectShow_Stream::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_I_DirectShow_Stream::IINITIALIZE_T*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (iinitialize_p);
      result_2 =
        iinitialize_p->initialize (directshow_ui_cb_data_p->configuration->streamConfiguration);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      istream_control_p = directshow_ui_cb_data_p->stream;
      Common_IGetR_2_T<Test_I_SpeechCommand_DirectShow_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_2_T<Test_I_SpeechCommand_DirectShow_SessionData_t>*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (iget_p);
      directshow_session_data_container_p = &iget_p->getR_2 ();
      directshow_session_data_p =
        &const_cast<Test_I_SpeechCommand_DirectShow_SessionData&> (directshow_session_data_container_p->getR ());
      session_data_p = directshow_session_data_p;
      directshow_ui_cb_data_p->progressData.sessionId =
        session_data_p->sessionId;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Test_I_MediaFoundation_Stream::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_I_MediaFoundation_Stream::IINITIALIZE_T*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (iinitialize_p);
      result_2 =
        iinitialize_p->initialize (mediafoundation_ui_cb_data_p->configuration->streamConfiguration);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
      istream_control_p = mediafoundation_ui_cb_data_p->stream;
      Common_IGetR_2_T<Test_I_SpeechCommand_MediaFoundation_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_2_T<Test_I_SpeechCommand_MediaFoundation_SessionData_t>*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (iget_p);
      mediafoundation_session_data_container_p = &iget_p->getR_2 ();
      mediafoundation_session_data_p =
        &const_cast<Test_I_SpeechCommand_MediaFoundation_SessionData&> (mediafoundation_session_data_container_p->getR ());
      session_data_p = mediafoundation_session_data_p;
      mediafoundation_ui_cb_data_p->progressData.sessionId =
        session_data_p->sessionId;
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
  Test_I_ALSA_Stream::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Test_I_ALSA_Stream::IINITIALIZE_T*> (ui_cb_data_p->stream);
  ACE_ASSERT (iinitialize_p);
  result_2 =
    iinitialize_p->initialize (ui_cb_data_p->configuration->streamConfiguration);
  istream_p = dynamic_cast<Stream_IStream_t*> (ui_cb_data_p->stream);
  istream_control_p = ui_cb_data_p->stream;
  Common_IGetR_2_T<Test_I_SpeechCommand_ALSA_SessionData_t>* iget_p =
    dynamic_cast<Common_IGetR_2_T<Test_I_SpeechCommand_ALSA_SessionData_t>*> (ui_cb_data_p->stream);
  ACE_ASSERT (iget_p);
  session_data_container_p = &iget_p->getR_2 ();
  session_data_p =
    &const_cast<Test_I_SpeechCommand_ALSA_SessionData&> (session_data_container_p->getR ());
  ui_cb_data_p->progressData.sessionId = session_data_p->sessionId;
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream: \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (istream_p);
  ACE_ASSERT (istream_control_p);
  ACE_ASSERT (session_data_p);

  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStreamControlBase::find(\"%s\"), aborting\n"),
                ACE_TEXT (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
    goto error;
  } // end IF
  dispatch_p =
    dynamic_cast<Common_IDispatch*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!dispatch_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_IDispatch*>(%@), aborting\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    goto error;
  } // end IF
  resize_notification_p =
    dynamic_cast<Test_I_Common_ISet_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!resize_notification_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Test_I_Common_ISet_t*>(%@), aborting\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    goto error;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (thread_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p->resizeNotification = resize_notification_p;
      directshow_ui_cb_data_p->spectrumAnalyzer = dispatch_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p->resizeNotification = resize_notification_p;
      mediafoundation_ui_cb_data_p->spectrumAnalyzer = dispatch_p;
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
  ui_cb_data_p->spectrumAnalyzer = dispatch_p;
#endif // ACE_WIN32 || ACE_WIN64

  // generate context id
  converter << session_data_p->sessionId;
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
  state_p->contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                              gtk_statusbar_get_context_id (statusbar_p,
                                                                            converter.str ().c_str ())));
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)

  istream_control_p->start ();
  //ACE_ASSERT (istream_control_p->isRunning ());
  istream_control_p->wait (true,   // wait for any worker thread(s) ?
                           false,  // wait for upstream (if any) ?
                           false); // wait for downstream (if any) ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  goto continue_;

error:
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
  event_source_id = g_idle_add (idle_session_end_cb,
                                thread_data_base_p->CBData);
  if (event_source_id == 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
  else
  { ACE_ASSERT (state_p);
    state_p->eventSourceIds.insert (event_source_id);
  } // end ELSE
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)

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
  if (COM_initialized) Common_Tools::finalizeCOM ();
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

/////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_2; // file writer
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_2; // file writer
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_3; // renderer
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_3; // renderer
  enum Stream_Device_Capturer capturer_e = STREAM_DEVICE_CAPTURER_INVALID;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->UIState);
      iterator =
        directshow_ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      ACE_ASSERT (iterator != directshow_ui_cb_data_p->UIState->builders.end ());
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      directshow_modulehandler_configuration_iterator_2 =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator_2 != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      directshow_modulehandler_configuration_iterator_3 =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator_3 != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      capturer_e =
        directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->UIState);
      iterator =
        mediafoundation_ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      ACE_ASSERT (iterator != mediafoundation_ui_cb_data_p->UIState->builders.end ());
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      mediafoundation_modulehandler_configuration_iterator_2 =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator_2 != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      mediafoundation_modulehandler_configuration_iterator_3 =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator_3 != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      capturer_e =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer;
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->UIState);
  iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator_2 = // renderer
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator_3 = // file writer
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator_3 != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  // step1: initialize widgets
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<double> (std::numeric_limits<uint32_t>::max ()));
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<double> (std::numeric_limits<uint32_t>::max ()));

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<double> (std::numeric_limits<uint32_t>::max ()));

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gint sort_column_id = 1; // device
  GtkSortType sort_order = GTK_SORT_DESCENDING;
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
  if (!load_capture_devices (list_store_p))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_capture_devices(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
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
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);

  GtkFileFilter* file_filter_p =
    GTK_FILE_FILTER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILEFILTER_WAV_NAME)));
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

  std::string filename_string;
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
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  filename_string =
    ((*modulehandler_configuration_iterator_3).second.second->fileIdentifier.empty () ? Common_File_Tools::getTempDirectory ()
                                                                                      : (*modulehandler_configuration_iterator_3).second.second->fileIdentifier.identifier);
  if (Common_File_Tools::isDirectory (filename_string))
    (*modulehandler_configuration_iterator_3).second.second->fileIdentifier.clear ();
#endif // ACE_WIN32 || ACE_WIN64
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  gboolean result = false;
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
          return G_SOURCE_REMOVE;
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
  //if (unlikely (!result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to gtk_file_chooser_set_file(\"%s\"): \"%m\", aborting\n"),
  //              ACE_TEXT (filename_string.c_str ())));
  //  return G_SOURCE_REMOVE;
  //} // end IF

  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode_2d =
    STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      mode_2d =
        (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mode_2d =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode;
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
    (*modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode;
#endif // ACE_WIN32 || ACE_WIN64
  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX));

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  gint tooltip_timeout_i =
      COMMON_UI_GTK_TIMEOUT_DEFAULT_WIDGET_TOOLTIP_DELAY_MS;
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,10,0)
#else
  g_object_set (G_OBJECT (drawing_area_p),
                ACE_TEXT_ALWAYS_CHAR ("gtk-tooltip-timeout"), &tooltip_timeout_i,
                NULL);
#endif // GTK_CHECK_VERSION (3,10,0)
#elif GTK_CHECK_VERSION(2,12,0) // *TODO*: this seems to be wrong
  g_object_set (G_OBJECT (drawing_area_p),
                ACE_TEXT_ALWAYS_CHAR ("gtk-tooltip-timeout"), &tooltip_timeout_i,
                NULL);
#endif // GTK_CHECK_VERSION (3,0,0) || GTK_CHECK_VERSION (2,12,0)

#if defined (GTKGL_SUPPORT)
  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_DISPLAY_NAME)));
  ACE_ASSERT (box_p);
  Common_UI_GTK_GLContextsIterator_t opengl_contexts_iterator;
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
  GtkGLArea* gl_area_p =
    GTK_GL_AREA (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_GLAREA_NAME)));
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
    GGLA_USE_GL,
// GGLA_BUFFER_SIZE
// GGLA_LEVEL
    GGLA_RGBA,
    GGLA_DOUBLEBUFFER,
//    GGLA_STEREO
//    GGLA_AUX_BUFFERS
    GGLA_RED_SIZE,   1,
    GGLA_GREEN_SIZE, 1,
    GGLA_BLUE_SIZE,  1,
    GGLA_ALPHA_SIZE, 1,
//    GGLA_DEPTH_SIZE
//    GGLA_STENCIL_SIZE
//    GGLA_ACCUM_RED_SIZE
//    GGLA_ACCUM_GREEN_SIZE
//    GGLA_ACCUM_BLUE_SIZE
//    GGLA_ACCUM_ALPHA_SIZE
//
//    GGLA_X_VISUAL_TYPE_EXT
//    GGLA_TRANSPARENT_TYPE_EXT
//    GGLA_TRANSPARENT_INDEX_VALUE_EXT
//    GGLA_TRANSPARENT_RED_VALUE_EXT
//    GGLA_TRANSPARENT_GREEN_VALUE_EXT
//    GGLA_TRANSPARENT_BLUE_VALUE_EXT
//    GGLA_TRANSPARENT_ALPHA_VALUE_EXT
    GGLA_NONE
  };

  GglaArea* gl_area_p = GGLA_AREA (ggla_area_new (attribute_list));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to ggla_area_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  ui_cb_data_base_p->UIState->OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                                     gl_area_p->glcontext));
  opengl_contexts_iterator =
    ui_cb_data_base_p->UIState->OpenGLContexts.find (gl_area_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (G_SOURCE_REMOVE);
  ACE_NOTREACHED (return G_SOURCE_REMOVE;)
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
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

  ui_cb_data_base_p->UIState->OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                                     gl_area_p->glcontext));
  opengl_contexts_iterator =
    ui_cb_data_base_p->UIState->OpenGLContexts.find (gl_area_p);
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
  ACE_ASSERT (opengl_contexts_iterator != ui_cb_data_base_p->UIState->OpenGLContexts.end ());

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK);
#else
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK    |
                         GDK_BUTTON_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#elif GTK_CHECK_VERSION (2,0,0)
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_events (GTK_WIDGET ((*opengl_contexts_iterator).first),
                         GDK_EXPOSURE_MASK    |
                         GDK_BUTTON_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (x,0,0) */

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
  //gtk_widget_set_visible (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */
  gtk_widget_set_size_request (GTK_WIDGET ((*opengl_contexts_iterator).first),
                               320, 240);

  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("realize"),
                    G_CALLBACK (glarea_realize_cb),
                    userData_in);

  //gtk_box_pack_start (box_p,
  //                    GTK_WIDGET ((*opengl_contexts_iterator).first),
  //                    TRUE, // expand
  //                    TRUE, // fill
  //                    0);   // padding
#if GTK_CHECK_VERSION (3,8,0)
//  gtk_builder_expose_object ((*iterator).second.second,
//                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GLAREA_3D_NAME),
//                             G_OBJECT ((*opengl_contexts_iterator).first));
#endif /* GTK_CHECK_VERSION (3,8,0) */
#endif /* GTKGL_SUPPORT */

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
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
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
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
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags> (GTK_RC_BASE |
                             GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  GtkTextIter iterator_3;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &iterator_3);
  gtk_text_buffer_create_mark (buffer_p,
                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCROLLMARK_NAME),
                               &iterator_3,
                               TRUE);
  g_object_unref (buffer_p);

  GtkWidget* about_dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

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

  result_2 = g_signal_connect_swapped (G_OBJECT (about_dialog_p),
                                       ACE_TEXT_ALWAYS_CHAR ("response"),
                                       G_CALLBACK (gtk_widget_hide),
                                       about_dialog_p);
  ACE_ASSERT (result_2);

#if defined (GTKGL_SUPPORT)
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

  // step9: draw main dialog
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  // step11: activate some widgets
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
  gint index_i = 0;
  gtk_combo_box_set_active (combo_box_p, index_i);

  bool is_active_b = false;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  switch (ui_cb_data_base_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//      is_active_b =
//        (*directshow_modulehandler_configuration_iterator).second.second->mute;
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//      is_active_b =
//        (*mediafoundation_modulehandler_configuration_iterator).second.second->mute;
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
//  is_active_b = (*modulehandler_configuration_iterator).second.second->mute;
//#endif // ACE_WIN32 || ACE_WIN64
  //GtkToggleButton* toggle_button_p =
  //  GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_MUTE_NAME)));
  //ACE_ASSERT (toggle_button_p);
  //gtk_toggle_button_set_active (toggle_button_p, is_active_b);

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  if (!filename_string.empty ())
  {
#if GTK_CHECK_VERSION(3,0,0)
    g_signal_handlers_block_by_func (G_OBJECT (toggle_button_p),
                                     G_CALLBACK (togglebutton_save_toggled_cb),
                                     userData_in);
#elif GTK_CHECK_VERSION(2, 0, 0)
    gtk_signal_handler_block_by_func (GTK_OBJECT (toggle_button_p),
                                      G_CALLBACK (togglebutton_save_toggled_cb),
                                      userData_in);
#endif // GTK_CHECK_VERSION(x,0,0)
    gtk_toggle_button_set_active (toggle_button_p, TRUE);
#if GTK_CHECK_VERSION(3,0,0)
    g_signal_handlers_unblock_by_func (G_OBJECT (toggle_button_p),
                                       G_CALLBACK (togglebutton_save_toggled_cb),
                                       userData_in);
#elif GTK_CHECK_VERSION(2,0,0)
    gtk_signal_handler_unblock_by_func (GTK_OBJECT (toggle_button_p),
                                        G_CALLBACK (togglebutton_save_toggled_cb),
                                        userData_in);
#endif // GTK_CHECK_VERSION(x,0,0)
  } // end IF

  GtkRadioButton* radio_button_p = NULL;
  is_active_b = (mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX);
  if (is_active_b)
  {
    toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME)));
    ACE_ASSERT (toggle_button_p);
    gtk_toggle_button_set_active (toggle_button_p, TRUE);

    radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                (mode_2d == STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE) ? ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME)
                                                                                                                       : ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_RADIOBUTTON_SPECTRUM_NAME)));
    ACE_ASSERT (radio_button_p);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p), TRUE);
  } // end IF

  GdkWindow* window_p = NULL;
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
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (!(*mediafoundation_modulehandler_configuration_iterator).second.second->window);
      (*mediafoundation_modulehandler_configuration_iterator).second.second->window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->window);
      window_p =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->window;
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
//  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second->window);
  (*modulehandler_configuration_iterator).second.second->window =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->window);
  window_p = (*modulehandler_configuration_iterator).second.second->window;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (window_p);

  // step12: initialize updates
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_base_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the info view
    guint event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                     idle_update_info_display_cb,
                     userData_in);
    if (event_source_id > 0)
      ui_cb_data_base_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

#if defined (GTKGL_SUPPORT)
    event_source_id =
      g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_OPENGL_MS,
                     idle_update_display_cb,
                     userData_in);
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_display_cb,
      //                 userData_in,
      //                 NULL);
    if (event_source_id > 0)
      ui_cb_data_base_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
#endif // GTKGL_SUPPORT
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      stream_p = directshow_ui_cb_data_p->stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      stream_p = mediafoundation_ui_cb_data_p->stream;
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
  stream_p = ui_cb_data_p->stream;
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
  if (ui_cb_data_p->handle)
  {
    result = snd_pcm_close (ui_cb_data_p->handle);
    if (unlikely (result < 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    ui_cb_data_p->handle = NULL;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_p->UIState->builders.end ());

  // *IMPORTANT NOTE*: there are three major reasons for being here that are not
  //                   mutually exclusive:
  //                   - user pressed stop
  //                   - audio file has ended playing
  //                   - there was an asynchronous error on the stream

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
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

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_SETTINGS_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  //button_p =
  //  GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                      ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_RESET_NAME)));
  //ACE_ASSERT (button_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  //toggle_button_p =
  //  GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_MUTE_NAME)));
  //ACE_ASSERT (toggle_button_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), TRUE);
  //button_p =
  //  GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                      ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_PROPERTIES_NAME)));
  //ACE_ASSERT (button_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
      static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
  GtkTextBuffer* text_buffer_p =
    GTK_TEXT_BUFFER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTBUFFER_NAME)));
  ACE_ASSERT (text_buffer_p);

  Stream_Decoder_DeepSpeech_Result_t* result_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      result_p = &directshow_ui_cb_data_p->progressData.words;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      result_p = &mediafoundation_ui_cb_data_p->progressData.words;
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  result_p = &ui_cb_data_p->progressData.words;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result_p);

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_base_p->UIState->lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (ui_cb_data_base_p->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_base_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      event_e));
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
    while (!ui_cb_data_base_p->UIState->eventStack.is_empty ())
    {
      result = ui_cb_data_base_p->UIState->eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE

    for (Stream_Decoder_DeepSpeech_ResultIterator_t iterator_2 = result_p->begin ();
         iterator_2 != result_p->end ();
         ++iterator_2)
    {
      gtk_text_buffer_insert_at_cursor (text_buffer_p,
                                        (*iterator_2).c_str (),
                                        (*iterator_2).size ());
      gtk_text_buffer_insert_at_cursor (text_buffer_p,
                                        ACE_TEXT_ALWAYS_CHAR (" "),
                                        1);
    } // end FOR
    result_p->clear ();
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  struct Test_I_UI_ProgressData* progress_data_p =
      static_cast<struct Test_I_UI_ProgressData*> (userData_in);
  ACE_ASSERT (progress_data_p);
  ACE_ASSERT (progress_data_p->state);
  Common_UI_GTK_BuildersIterator_t iterator =
    progress_data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != progress_data_p->state->builders.end ());

  int result = -1;

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  bool done = false;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, progress_data_p->state->lock, G_SOURCE_REMOVE);
    for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = progress_data_p->completedActions.begin ();
         iterator_3 != progress_data_p->completedActions.end ();
         ++iterator_3)
    {
      iterator_2 = progress_data_p->pendingActions.find (*iterator_3);
      ACE_ASSERT (iterator_2 != progress_data_p->pendingActions.end ());
      result = thread_manager_p->join ((*iterator_2).second.id (), &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    (*iterator_2).second.id ()));
      else
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("thread %u has joined (status was: %u)\n"),
                    (*iterator_2).second.id (),
                    exit_status));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("thread %u has joined (status was: %@)\n"),
                    (*iterator_2).second.id (),
                    exit_status));
#endif // ACE_WIN32 || ACE_WIN64
      } // end ELSE

      progress_data_p->state->eventSourceIds.erase (*iterator_3);
      progress_data_p->pendingActions.erase (iterator_2);
    } // end FOR
    progress_data_p->completedActions.clear ();

    if (progress_data_p->pendingActions.empty ())
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
  } // end lock scope

  std::ostringstream converter;
  converter << progress_data_p->statistic.messagesPerSecond;
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

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator = state_r.builders.find (
    ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GdkWindow* window_p = NULL;
#if defined (GTKGL_SUPPORT)
  Common_UI_GTK_GLContextsIterator_t iterator_2;
#endif /* GTKGL_SUPPORT */

  // trigger refresh of the 2D area
  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  if (unlikely (!window_p))
    goto continue_2; // <-- not realized yet

  gdk_window_invalidate_rect (window_p,
                              NULL,   // whole window
                              FALSE); // invaliddate children ?

#if defined (GTKGL_SUPPORT)
  // trigger refresh of the 3D OpenGL area
  ACE_ASSERT (!state_r.OpenGLContexts.empty ());
  iterator_2 = state_r.OpenGLContexts.begin ();
  window_p = gtk_widget_get_window (GTK_WIDGET (&(*iterator_2).first->darea));
  if (unlikely (!window_p))
    goto continue_2; // <-- not realized yet

  gdk_window_invalidate_rect (window_p,
                              NULL,   // whole window
                              FALSE); // invaliddate children ?
#endif /* GTKGL_SUPPORT */
continue_2:
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
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
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
  //bool is_file_source_b = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
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
      //is_file_source_b =
      //  (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType == AUDIOEFFECT_SOURCE_FILE);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
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
      //is_file_source_b =
      //  (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType == AUDIOEFFECT_SOURCE_FILE);
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  stream_p = ui_cb_data_p->stream;
  //is_file_source_b =
  //  (ui_cb_data_p->configuration->streamConfiguration.configuration_->sourceType == AUDIOEFFECT_SOURCE_FILE);
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
    stream_p->stop (false,  // wait for completion ?
                    false,  // recurse upstream ?
                    false); // high priority ?
                    //is_file_source_b); // high priority ?

    return;
  } // end IF

  // --> user pressed record

  struct Test_I_UI_ThreadData* thread_data_p = NULL;
  ACE_TCHAR thread_name[BUFSIZ];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  struct Test_I_SpeechCommand_UI_ProgressData* progress_data_p = NULL;
//  GtkSpinButton* spin_button_p = NULL;
//  unsigned int value_i = 0;

  // step2: modify widgets
  gtk_button_set_label (GTK_BUTTON (toggleButton_in), GTK_STOCK_MEDIA_STOP);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_SETTINGS_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_RESET_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  //GtkToggleButton* toggle_button_p =
  //  GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_MUTE_NAME)));
  //ACE_ASSERT (toggle_button_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), FALSE);

  // step1: set up progress reporting
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
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
      struct Test_I_DirectShow_UI_ThreadData* thread_data_2 = NULL;
      ACE_NEW_NORETURN (thread_data_2,
                        struct Test_I_DirectShow_UI_ThreadData ());
      ACE_ASSERT (thread_data_2);
      thread_data_2->CBData = directshow_ui_cb_data_p;
      thread_data_p = thread_data_2;
      progress_data_p = &directshow_ui_cb_data_p->progressData;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      struct Test_I_MediaFoundation_UI_ThreadData* thread_data_2 =
        NULL;
      ACE_NEW_NORETURN (thread_data_2,
                        struct Test_I_MediaFoundation_UI_ThreadData ());
      ACE_ASSERT (thread_data_2);
      thread_data_2->CBData = mediafoundation_ui_cb_data_p;
      thread_data_2->mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
      thread_data_p = thread_data_2;
      progress_data_p = &mediafoundation_ui_cb_data_p->progressData;
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
                    struct Test_I_ALSA_UI_ThreadData);
  ACE_ASSERT (thread_data_p);
  static_cast<struct Test_I_ALSA_UI_ThreadData*> (thread_data_p)->CBData =
    ui_cb_data_p;
  progress_data_p = &ui_cb_data_p->progressData;
#endif // ACE_WIN32 || ACE_WIN64
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (progress_data_p);
  // progress_data_p->bytesPerFrame = bytes_per_frame_i;
  progress_data_p->statistic = Test_I_Statistic ();

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
//                  ACE_TEXT (TEST_U_Test_I_THREAD_NAME));
//  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_I_STREAM_THREAD_NAME));
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
    progress_data_p->eventSourceId =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_update_progress_cb,
                       progress_data_p,
                       NULL);
      //g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
      //                    COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
      //                    idle_update_progress_cb,
      //                    &ui_cb_data_base_p->progressData,
      //                    NULL);
    if (!progress_data_p->eventSourceId)
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
    thread_data_p->eventSourceId = progress_data_p->eventSourceId;
    progress_data_p->pendingActions[progress_data_p->eventSourceId] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    state_r.eventSourceIds.insert (progress_data_p->eventSourceId);
  } // end lock scope
} // togglebutton_record_toggled_cb

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
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p = NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  bool use_framework_source_b = false;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
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
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  stream_p = ui_cb_data_p->stream;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  std::string device_identifier_string;
  unsigned int card_id_i = std::numeric_limits<unsigned int>::max ();
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value, value_2;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
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

//  gint n_rows = 0;
  GtkToggleButton* toggle_button_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT && (_WIN32_WINNT >= 0x0602)
  //std::string format_string;
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
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          (*directshow_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          (*directshow_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid =
            Common_Tools::StringToGUID (device_identifier_string);
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

      //format_string =
      //  Common_Tools::GUIDToString (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.subtype);

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
      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->Release (); mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format = NULL;

      switch (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
        {
          (*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          (*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._id =
            card_id_i;
          struct tWAVEFORMATEX waveformatex_s;
          ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
          Stream_MediaFramework_DirectSound_Tools::getBestFormat (card_id_i,
                                                                  waveformatex_s);
          mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format =
            Stream_MediaFramework_MediaFoundation_Tools::to (waveformatex_s);

          break;
        }
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          (*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          (*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid =
            Common_Tools::StringToGUID (device_identifier_string);
          struct tWAVEFORMATEX* waveformatex_p =
            Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat ((*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid);
          if (unlikely (!waveformatex_p))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat(), returning\n")));
            return;
          } // end IF
          mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format =
            Stream_MediaFramework_MediaFoundation_Tools::to (*waveformatex_p);
          CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;

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
      if (unlikely (!mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::to(), returning\n")));
        return;
      } // end IF

      //struct _GUID GUID_2 = GUID_NULL;
      //HRESULT result_3 =
      //  mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->GetGUID (MF_MT_SUBTYPE,
      //                                                                                                    &GUID_2);
      //if (FAILED (result_3))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result_3).c_str ())));
      //  return;
      //} // end IF
      //format_string = Common_Tools::GUIDToString (GUID_2);

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
      if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid,
                                                                        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                        media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(\"%s\"), returning\n"),
                    ACE_TEXT (Common_Tools::GUIDToString ((*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid).c_str ())));
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
#else
  ACE_UNUSED_ARG (card_id_i);
  (*modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier =
      device_identifier_string;
#endif // ACE_WIN32 || ACE_WIN64

//  bool result_2 = false;
  GtkScale* hscale_p = NULL, *hscale_2 = NULL;
  std::ostringstream converter;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Mic_Source_DirectShow* directshow_source_impl_p = NULL;
  Test_I_DirectShow_Source* directshow_source_impl_2 = NULL;
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
        case STREAM_DEVICE_CAPTURER_WASAPI:
          goto continue_;
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
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
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      switch (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        case STREAM_DEVICE_CAPTURER_WASAPI:
          goto continue_;
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer (was: %d), returning\n"),
                      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->capturer));
          return;
        }
      } // end SWITCH

      Common_IGetR_5_T<Test_I_Mic_Source_MediaFoundation>* iget_p =
          dynamic_cast<Common_IGetR_5_T<Test_I_Mic_Source_MediaFoundation>*> (stream_p);
      ACE_ASSERT (iget_p);
      Test_I_Mic_Source_MediaFoundation* writer_p =
          &const_cast<Test_I_Mic_Source_MediaFoundation&> (iget_p->getR_5 ());
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
  int mode = STREAM_LIB_ALSA_CAPTURE_DEFAULT_MODE;
//   if ((*modulehandler_configuration_iterator).second.second->ALSAConfiguration->asynch)
//     mode |= SND_PCM_ASYNC;
  result = snd_pcm_open (&ui_cb_data_p->handle,
                         device_identifier_string.c_str (),
                         SND_PCM_STREAM_CAPTURE, mode);
  if ((result < 0) || !ui_cb_data_p->handle)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_open(\"%s\",%d) for capture: \"%s\", aborting\n"),
                ACE_TEXT (device_identifier_string.c_str ()),
                mode,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  (*modulehandler_configuration_iterator).second.second->ALSAConfiguration->handle =
    ui_cb_data_p->handle;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened ALSA device (capture) \"%s\"\n"),
              ACE_TEXT (device_identifier_string.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), TRUE);

  // get/set capture volume / boost levels
  hscale_p =
        GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_HSCALE_VOLUME_NAME)));
  ACE_ASSERT (hscale_p);
  hscale_2 =
          GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_HSCALE_BOOST_NAME)));
  ACE_ASSERT (hscale_2);
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
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ()),
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
  if (unlikely (!i_audio_volume_level_p))
  { // *NOTE*: the stereo mix device does not have a boost control
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getMicrophoneBoostControl(\"%s\") (device id was: %u), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ()),
                card_id_i));
    goto continue_2;
  } // end IF
  result = i_audio_volume_level_p->GetLevelRange (0,
                                                  &min_level_f,
                                                  &max_level_f,
                                                  &stepping_f);
  ACE_ASSERT (SUCCEEDED (result));
  gtk_range_set_range (GTK_RANGE (hscale_2),
                       static_cast<gdouble> (min_level_f),
                       static_cast<gdouble> (max_level_f));
  gtk_range_set_increments (GTK_RANGE (hscale_2),
                            static_cast<gdouble> (stepping_f),
                            static_cast<gdouble> (stepping_f));
  converter.precision (0);
  converter << std::fixed; // for fixed point notation
  for (float i = min_level_f;
       i <= max_level_f;
       i += stepping_f)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << i;
    gtk_scale_add_mark (GTK_SCALE (hscale_2),
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
      (*directshow_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid =
        GUID_s;
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
      (*mediafoundation_modulehandler_configuration_iterator).second.second->deviceIdentifier.identifier._guid =
        GUID_s;
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
      return;
    }
  } // end SWITCH
  gtk_range_set_value (GTK_RANGE (hscale_p),
                       static_cast<gdouble> (volume_level_f) * 100.0);
  gtk_range_set_value (GTK_RANGE (hscale_2),
                       static_cast<gdouble> (boost_f));
#else
  if (!Stream_MediaFramework_ALSA_Tools::getVolumeLevels (device_identifier_string,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_VOLUME_NAME),
                                                          true, // capture
                                                          min_level_i,
                                                          max_level_i,
                                                          current_level_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getVolumeLevels(\"%s\",\"%s\"), returning\n"),
                ACE_TEXT (device_identifier_string.c_str ()),
                ACE_TEXT (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_VOLUME_NAME)));
    return;
  } // end IF
  gtk_range_set_range (GTK_RANGE (hscale_p),
                       static_cast<gdouble> (min_level_i),
                       static_cast<gdouble> (max_level_i));
  gtk_range_set_increments (GTK_RANGE (hscale_p),
                            static_cast<gdouble> (1),
                            static_cast<gdouble> (1));
  gtk_range_set_value (GTK_RANGE (hscale_p),
                       static_cast<gdouble> (current_level_i));

  if (!Stream_MediaFramework_ALSA_Tools::getVolumeLevels (device_identifier_string,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME),
                                                          true, // capture
                                                          min_level_i,
                                                          max_level_i,
                                                          current_level_i))
  {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getVolumeLevels(\"%s\",\"%s\"), returning\n"),
                 ACE_TEXT (device_identifier_string.c_str ()),
                 ACE_TEXT (STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME)));
      return;
  } // end IF
  gtk_range_set_range (GTK_RANGE (hscale_2),
                       static_cast<gdouble> (min_level_i),
                       static_cast<gdouble> (max_level_i));
  gtk_range_set_increments (GTK_RANGE (hscale_2),
                            static_cast<gdouble> (1),
                            static_cast<gdouble> (1));
  for (long i = min_level_i;
       i <= max_level_i;
       i += 1)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << i;
    gtk_scale_add_mark (GTK_SCALE (hscale_2),
                        static_cast<gdouble> (i),
                        GTK_POS_TOP,
                        converter.str ().c_str ());
  } // end FOR
  gtk_range_set_value (GTK_RANGE (hscale_2),
                       static_cast<gdouble> (current_level_i));
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
togglebutton_save_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_save_toggled_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersConstIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
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
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
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
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
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
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (file_chooser_button_p),
                            is_active);
} // togglebutton_save_toggled_cb

void
togglebutton_visualization_toggled_cb (GtkToggleButton* toggleButton_in,
                                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_visualization_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
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
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_VISUALIZATION_NAME)));
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
  if (!gtk_toggle_button_get_active (toggleButton_in))
    return;
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkRadioButton* radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME)));
  ACE_ASSERT (radio_button_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode =
          (radio_button_p == GTK_RADIO_BUTTON (toggleButton_in) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE
                                                                : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode =
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
  (*modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode =
      (radio_button_p == GTK_RADIO_BUTTON (toggleButton_in) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE
                                                            : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM);
#endif // ACE_WIN32 || ACE_WIN64
} // radiobutton_2d_toggled_cb

//void
//button_clear_clicked_cb (GtkWidget* widget_in,
//                         gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));
//
//  ACE_UNUSED_ARG (widget_in);
//
//  // sanity check(s)
//  struct Test_I_UI_CBData* ui_cb_data_base_p =
//    static_cast<struct Test_I_UI_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_base_p);
//  ACE_ASSERT (ui_cb_data_base_p->UIState);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
//
//  GtkTextView* view_p =
//    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
//                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
//  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//    gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
//  gtk_text_buffer_set_text (buffer_p,
//                            ACE_TEXT_ALWAYS_CHAR (""), 0);
//}

void
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (dialog_p);

  // run dialog
  gint result = gtk_dialog_run (dialog_p);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  //gtk_widget_hide (GTK_WIDGET (dialog_p));
} // button_about_clicked_cb

void
button_settings_clicked_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_settings_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
} // button_settings_clicked_cb

void
button_reset_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_reset_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
} // button_reset_clicked_cb

void
hscale_volume_value_changed_cb (GtkRange* range_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_volume_value_changed_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

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
} // hscale_volume_value_changed_cb

gboolean
hscale_boost_change_value_cb (GtkRange* range_in,
                              GtkScrollType* scrollType_in,
                              gdouble value_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_boost_change_value_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->boostControl);
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
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      if (!mediafoundation_ui_cb_data_p->boostControl)
        return FALSE; // propagate the event
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

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
} // hscale_boost_change_value_cb

void
hscale_boost_value_changed_cb (GtkRange* range_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::hscale_boost_value_changed_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      if (unlikely (!directshow_ui_cb_data_p->boostControl))
        break;
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
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      if (unlikely (!mediafoundation_ui_cb_data_p->boostControl))
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
  // sanity check(s)
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

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
} // hscale_boost_value_changed_cb

void
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
  enum Stream_StateMachine_ControlState status_e = STREAM_STATE_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      stream_p = directshow_ui_cb_data_p->stream;
      Test_I_DirectShow_IStreamControlBase_t* istream_control_p =
        dynamic_cast<Test_I_DirectShow_IStreamControlBase_t*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (istream_control_p);
      status_e = istream_control_p->status ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      stream_p = mediafoundation_ui_cb_data_p->stream;
      Test_I_MediaFoundation_IStreamControlBase_t* istream_control_p =
        dynamic_cast<Test_I_MediaFoundation_IStreamControlBase_t*> (mediafoundation_ui_cb_data_p->stream);
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
  // sanity check(s)
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT (ui_cb_data_p->stream);
  stream_p = ui_cb_data_p->stream;
  Test_I_ALSA_IStreamControlBase_t* istream_control_p =
    dynamic_cast<Test_I_ALSA_IStreamControlBase_t*> (ui_cb_data_p->stream);
  ACE_ASSERT (istream_control_p);
  status_e = istream_control_p->status ();
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  if ((status_e == STREAM_STATE_RUNNING) ||
      (status_e == STREAM_STATE_PAUSED))
    stream_p->stop (false, // wait for completion ?
                    false, // recurse upstream ?
                    true); // high priority ?

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
textview_size_allocate_cb (GtkWidget* widget_in,
                           GdkRectangle* rectangle_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::textview_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (rectangle_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
      static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());

  GtkScrolledWindow* scrolled_window_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCROLLEDWINDOW_NAME)));
  ACE_ASSERT (scrolled_window_p);
  GtkAdjustment* adjustment_p =
    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p) - gtk_adjustment_get_page_size (adjustment_p));
} // textview_size_allocate_cb

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
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStream_t* istream_p = NULL;
  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode =
      STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID;
  unsigned int sample_size = 0; // bytes
  bool is_signed_format = true;
  unsigned int channels = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  HRESULT result = E_FAIL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      if (!directshow_ui_cb_data_p->stream->isRunning ())
        return FALSE;
      istream_p = dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      mode =
        (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode;
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->outputFormat.cbFormat == sizeof (struct tWAVEFORMATEX));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> ((*directshow_modulehandler_configuration_iterator).second.second->outputFormat.pbFormat);
      ACE_ASSERT (waveformatex_p);
      sample_size = waveformatex_p->wBitsPerSample / 8;
      channels = waveformatex_p->nChannels;
      // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
      //         are unsigned values. (Each audio sample has the range 0�255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      is_signed_format = !(sample_size == 1);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      if (!mediafoundation_ui_cb_data_p->stream->isRunning ())
        return FALSE;
      istream_p = dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
      mode =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode;
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
      //         are unsigned values. (Each audio sample has the range 0�255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      is_signed_format = !(sample_size == 1);
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
  if (!ui_cb_data_p->stream->isRunning ())
    return FALSE;
  istream_p = dynamic_cast<Stream_IStream_t*> (ui_cb_data_p->stream);
  mode =
    (*modulehandler_configuration_iterator).second.second->spectrumAnalyzer2DMode;
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
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
    return FALSE;
  } // end IF
  Common_Math_FFT_Double_t* math_fft_p =
    dynamic_cast<Common_Math_FFT_Double_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!math_fft_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_Math_FFT_T<double>*>(%@), returning\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    return FALSE;
  } // end IF

  GtkAllocation allocation;
  gtk_widget_get_allocation (widget_in,
                             &allocation);
  double half_height = allocation.height / 2.0;
  uint64_t maximum_value = Common_Tools::max<uint64_t> (sample_size,
                                                        is_signed_format);
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
      if (is_signed_format)
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
} // drawingarea_query_tooltip_cb

void
drawingarea_realize_cb (GtkWidget* widget_in,
                        gpointer   userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_realize_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (userData_in);
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  //GtkAllocation allocation;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
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

  ACE_UNUSED_ARG (allocation_in);

  // sanity check(s)
  struct Test_I_SpeechCommand_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_SpeechCommand_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  if (!window_p)
    return; // <-- not realized yet
  if (!ui_cb_data_base_p->resizeNotification)
    return; // stream not running (yet)

  try {
    ui_cb_data_base_p->resizeNotification->setP (window_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_ISetP_T::setP(), continuing\n")));
  }
} // drawingarea_size_allocate_cb

#if GTK_CHECK_VERSION (3,0,0)
gboolean
drawingarea_configure_event_cb (GtkWidget* widget_in,
                                GdkEvent* event_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  if (!window_p)
    return FALSE; // not realized yet
  struct Test_I_SpeechCommand_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_SpeechCommand_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  if (!ui_cb_data_base_p->resizeNotification)
    return FALSE; // stream not running (yet)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  if (widget_in != GTK_WIDGET (drawing_area_p))
    return FALSE;

  try {
    ui_cb_data_base_p->resizeNotification->setP (window_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_ISetP_T::setP(), continuing\n")));
  }

  return FALSE;
} // drawingarea_configure_event_cb

gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  // sanity check(s)
  struct Test_I_SpeechCommand_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_SpeechCommand_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ui_cb_data_base_p->spectrumAnalyzerCBData.context = context_in;
  ui_cb_data_base_p->spectrumAnalyzerCBData.window =
    gtk_widget_get_window (widget_in);
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.window)
    return FALSE; // not realized yet
  if (!ui_cb_data_base_p->spectrumAnalyzer)
    return FALSE; // stream not running (yet)
  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      stream_p = directshow_ui_cb_data_p->stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
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
      return FALSE;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->stream);
  stream_p = ui_cb_data_p->stream;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);
  if (!stream_p->isRunning ())
    return FALSE; // stream not running (yet)

  try {
    ui_cb_data_base_p->spectrumAnalyzer->dispatch (&ui_cb_data_base_p->spectrumAnalyzerCBData);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  return TRUE;
} // drawingarea_draw_cb
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
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  //// sanity check(s)
  //if (!ui_cb_data_base_p->pixelBuffer2D)
  //  return FALSE; // --> widget has not been realized yet

  //cairo_t* context_p =
  //  gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget_in)));
  //if (unlikely (!context_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
  //  return FALSE;
  //} // end IF
  //gdk_cairo_set_source_pixbuf (context_p,
  //                             ui_cb_data_base_p->pixelBuffer2D,
  //                             0.0, 0.0);

  //cairo_paint (context_p);

  //cairo_destroy (context_p); context_p = NULL;

  return TRUE;
} // drawingarea_expose_event_cb
#endif // GTK_CHECK_VERSION(3,0,0)

void
filechooserbutton_save_file_set_cb (GtkFileChooserButton* button_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_save_file_set_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
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
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second->fileIdentifier.identifier =
    Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
#endif // ACE_WIN32 || ACE_WIN64
  g_free (filename_p);
} // filechooserbutton_save_file_set_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */