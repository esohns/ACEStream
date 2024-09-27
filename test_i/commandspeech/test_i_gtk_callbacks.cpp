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

#include "common_os_tools.h"

#include "common_timer_manager.h"

#include "common_ui_tools.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#if defined(ACE_WIN32) || defined(ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_i_commandspeech_defines.h"
#include "test_i_stream.h"

// global variables
bool untoggling_record_button = false;

int
acestream_test_i_commandspeech_selector (const dirent* dirEntry_in)
{
  //STREAM_TRACE (ACE_TEXT ("acestream_test_i_commandspeech_selector"));

  // *NOTE*: select only files following the naming schema for
  //         voice files: "*.flitevox"

  // sanity check --> suffix ok ?
  std::string file_extension =
      Common_File_Tools::fileExtension (ACE_TEXT_ALWAYS_CHAR (dirEntry_in->d_name),
                                        true); // return leading '.'
  if (ACE_OS::strncmp (file_extension.c_str (),
                       ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_FLITE_VOICE_FILENAME_EXTENSION_STRING),
                       ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_FLITE_VOICE_FILENAME_EXTENSION_STRING))))
    return 0;

  return 1;
}

bool
load_voices (GtkListStore* listStore_in,
             const std::string& voicesDirectory_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_voices"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  Common_File_IdentifierList_t files_a =
    Common_File_Tools::files (voicesDirectory_in,
                              acestream_test_i_commandspeech_selector);
  std::string filename_string;
  for (Common_File_IdentifierListIterator_t iterator_2 = files_a.begin ();
       iterator_2 != files_a.end ();
       ++iterator_2)
  {
    filename_string =
      Common_File_Tools::basename ((*iterator_2).identifier, true);
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, filename_string.c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_playback_devices (GtkListStore* listStore_in,
                       enum Stream_Device_Renderer renderer_in)
#else
load_playback_devices (GtkListStore* listStore_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::load_playback_devices"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  std::string device_identifier_string;
  std::string device_friendlyname_string;
  UINT device_id_i = 0;
  switch (renderer_in)
  {
    case STREAM_DEVICE_RENDERER_WAVEOUT:
    {
      MMRESULT result_3 = MMSYSERR_NOERROR;
      WAVEOUTCAPS capabilities_s;
      UINT num_devices_i = waveOutGetNumDevs ();
      for (UINT i = 0;
           i < num_devices_i;
           ++i)
      {
        result_3 = waveOutGetDevCaps (i,
                                      &capabilities_s,
                                      sizeof (WAVEOUTCAPS));
        if (unlikely (result_3 != MMSYSERR_NOERROR))
        { char error_msg_a[BUFSIZ];
          waveInGetErrorText (result_3, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to waveOutGetDevCaps(%d): \"%s\", aborting\n"),
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
                                                                                                                 false)); // playback
        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, ACE_TEXT_ALWAYS_CHAR (capabilities_s.szPname),
                            1, i,
                            2, device_identifier_string.c_str (),
                            -1);
      } // end FOR

      result = true;
      break;
    }
    case STREAM_DEVICE_RENDERER_WASAPI:
    {
      IMMDeviceEnumerator* enumerator_p = NULL;
      result_2 =
        CoCreateInstance (__uuidof (MMDeviceEnumerator), NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS (&enumerator_p));
      ACE_ASSERT (SUCCEEDED (result_2) && enumerator_p);
      IMMDeviceCollection* devices_p = NULL;
      result_2 = enumerator_p->EnumAudioEndpoints (eRender,
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
        if (SUCCEEDED (result_2))
        { ACE_ASSERT (property_s.vt == VT_LPWSTR);
          device_friendlyname_string =
            ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (property_s.pwszVal));
        } // end IF
        else
          device_friendlyname_string.clear ();
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
        if (unlikely (device_id_i == std::numeric_limits<unsigned int>::max ()))
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId(%s), continuing\n"),
                      ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ())));
          continue;
        } // end IF

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, device_friendlyname_string.c_str (),
                            1, device_id_i,
                            2, device_identifier_string.c_str (),
                            -1);
      } // end FOR
      devices_p->Release (); devices_p = NULL;

      result = true;
      break;
    }
    case STREAM_DEVICE_RENDERER_DIRECTSHOW:
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
        enumerator_p->CreateClassEnumerator (CLSID_AudioRendererCategory,
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
        result_2 = moniker_p->BindToStorage (NULL, // *TODO*: CreateBindCtx() here ?
                                             NULL,
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
                            1, device_id_i,
                            2, device_path.c_str (),
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
    case STREAM_DEVICE_RENDERER_MEDIAFOUNDATION:
    {
      IMFAttributes* attributes_p = NULL;
      IMFActivate** devices_pp = NULL;
      UINT32 item_count = 0;
      //WCHAR buffer_a[BUFSIZ];
      UINT32 length = 0;
      std::string device_endpointid_string;

      ACE_ASSERT (false); // *TODO*
//      result_2 = MFCreateAttributes (&attributes_p, 1);
//      if (FAILED (result_2) || !attributes_p)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
//                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
//        return false;
//      } // end IF
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
//      result_2 =
//        attributes_p->SetGUID (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
//                               MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
//      if (FAILED (result_2))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to IMFAttributes::SetGUID(): \"%s\", aborting\n"),
//                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
//        goto error_2;
//      } // end IF
//      result_2 = MFEnumDeviceSources (attributes_p,
//                                      &devices_pp,
//                                      &item_count);
//      if (FAILED (result_2) || !devices_pp)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
//                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
//        goto error_2;
//      } // end IF
//#else
//      ACE_ASSERT (false);
//      ACE_NOTSUP_RETURN (false);
//      ACE_NOTREACHED (return false;)
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
//      attributes_p->Release (); attributes_p = NULL;
      for (UINT32 index = 0; index < item_count; index++)
      {
//        ACE_OS::memset (buffer_a, 0, sizeof (WCHAR[BUFSIZ]));
//        length = 0;
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
//        result_2 =
//          devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
//                                        buffer_a,
//                                        sizeof (WCHAR[BUFSIZ]),
//                                        &length);
//#else
//        ACE_ASSERT (false); // *TODO*
//        ACE_NOTSUP_RETURN (false);
//        ACE_NOTREACHED (return false;)
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
//        if (FAILED (result_2))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME): \"%s\", aborting\n"),
//                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//          goto error_2;
//        } // end IF
//        device_friendlyname_string =
//          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));
//        length = 0;
//        result_2 =
//          devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID,
//                                        buffer_a,
//                                        sizeof (WCHAR[BUFSIZ]),
//                                        &length);
//        if (FAILED (result_2))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID): \"%s\", aborting\n"),
//                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//          goto error_2;
//        } // end IF
//        device_endpointid_string =
//          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));
//        device_id_i =
//          Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (Stream_MediaFramework_DirectSound_Tools::endpointIdToDirectSoundGUID (device_endpointid_string));
//        //ACE_DEBUG ((LM_DEBUG,
//        //            ACE_TEXT ("found device \"%s\": \"%s\"\n"),
//        //            ACE_TEXT (device_friendlyname_string.c_str ()),
//        //            ACE_TEXT (device_endpointid_string.c_str ())));

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, device_friendlyname_string.c_str (),
                            1, device_id_i,
                            2, device_endpointid_string.c_str (),
                            -1);
      } // end FOR

      //for (UINT32 i = 0; i < item_count; i++)
      //  devices_pp[i]->Release ();
      //CoTaskMemFree (devices_pp); devices_pp = NULL;

      result = true;
      break;

//error_2:
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
                  ACE_TEXT ("invalid/unknown renderer type (was: %d), aborting\n"),
                  renderer_in));
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
      if (ACE_OS::strcmp (string_2, ACE_TEXT_ALWAYS_CHAR ("Output")))
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

bool
load_display_adapters (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_display_adapters"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  Common_UI_DisplayAdapters_t adapters_a =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Common_UI_Tools::getAdapters (true); // exclude 'mirroring' devices
#else
    Common_UI_Tools::getAdapters ();
#endif // ACE_WIN32 || ACE_WIN64
  for (Common_UI_DisplayAdaptersIterator_t iterator_2 = adapters_a.begin ();
       iterator_2 != adapters_a.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).description.c_str (),
                        1, (*iterator_2).device.c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
load_displays (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_displays"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  Common_UI_DisplayDevices_t displays_a = Common_UI_Tools::getDisplays ();
  for (Common_UI_DisplayDevicesIterator_t iterator_2 = displays_a.begin ();
       iterator_2 != displays_a.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).description.c_str (),
                        1, (*iterator_2).device.c_str (),
                        -1);
  } // end FOR

  return true;
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
  struct Test_I_CommandSpeech_UI_ThreadData* thread_data_base_p =
    static_cast<struct Test_I_CommandSpeech_UI_ThreadData*> (arg_in);
  ACE_ASSERT (thread_data_base_p);

  Common_UI_GTK_BuildersConstIterator_t iterator;
  Common_UI_GTK_State_t* state_p = NULL;
  std::ostringstream converter;
  Test_I_SessionData* session_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  const Test_I_CommandSpeech_DirectShow_SessionData_t* directshow_session_data_container_p =
    NULL;
  Test_I_CommandSpeech_DirectShow_SessionData* directshow_session_data_p =
    NULL;
  const Test_I_CommandSpeech_MediaFoundation_SessionData_t* mediafoundation_session_data_container_p =
    NULL;
  Test_I_CommandSpeech_MediaFoundation_SessionData* mediafoundation_session_data_p =
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
  const Test_I_CommandSpeech_ALSA_SessionData_t* session_data_container_p =
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
      Common_IGetR_2_T<Test_I_CommandSpeech_DirectShow_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_2_T<Test_I_CommandSpeech_DirectShow_SessionData_t>*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (iget_p);
      directshow_session_data_container_p = &iget_p->getR_2 ();
      directshow_session_data_p =
        &const_cast<Test_I_CommandSpeech_DirectShow_SessionData&> (directshow_session_data_container_p->getR ());
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
      Common_IGetR_2_T<Test_I_CommandSpeech_MediaFoundation_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_2_T<Test_I_CommandSpeech_MediaFoundation_SessionData_t>*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (iget_p);
      mediafoundation_session_data_container_p = &iget_p->getR_2 ();
      mediafoundation_session_data_p =
        &const_cast<Test_I_CommandSpeech_MediaFoundation_SessionData&> (mediafoundation_session_data_container_p->getR ());
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
  Common_IGetR_2_T<Test_I_CommandSpeech_ALSA_SessionData_t>* iget_p =
    dynamic_cast<Common_IGetR_2_T<Test_I_CommandSpeech_ALSA_SessionData_t>*> (ui_cb_data_p->stream);
  ACE_ASSERT (iget_p);
  session_data_container_p = &iget_p->getR_2 ();
  session_data_p =
    &const_cast<Test_I_CommandSpeech_ALSA_SessionData&> (session_data_container_p->getR ());
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
  g_source_remove (thread_data_base_p->displayEventSourceId);
  g_source_remove (thread_data_base_p->infoEventSourceId);

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
  std::string voices_directory_string, voice_string;
  bool mute_b = false;
#if defined(ACE_WIN32) || defined(ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_2; // file writer
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_2; // file writer
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_3; // renderer
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_3; // renderer
  enum Stream_Device_Renderer renderer_e = STREAM_DEVICE_RENDERER_INVALID;
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

      renderer_e =
        directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer;
      voices_directory_string =
        (*directshow_modulehandler_configuration_iterator).second.second->voiceDirectory;
      voice_string =
        (*directshow_modulehandler_configuration_iterator).second.second->voice;
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      mute_b =
        ((*directshow_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifier._id == -1);
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

      renderer_e =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer;
      voices_directory_string =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->voiceDirectory;
      voice_string =
        (*mediafoundation_modulehandler_configuration_iterator).second.second->voice;
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      mute_b =
        InlineIsEqualGUID ((*mediafoundation_modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifier._guid, GUID_NULL);
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

  voices_directory_string =
    (*modulehandler_configuration_iterator).second.second->voiceDirectory;
  voice_string =
    (*modulehandler_configuration_iterator).second.second->voice;
  mute_b =
    (*modulehandler_configuration_iterator_3).second.second->deviceIdentifier.identifier.empty ();
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_VOICE_NAME)));
  ACE_ASSERT (list_store_p);
  if (!load_voices (list_store_p,
                    voices_directory_string))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_voices(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_VOICE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
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

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_TARGET_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, // device id
                                        GTK_SORT_ASCENDING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_playback_devices (list_store_p,
                              renderer_e))
#else
  if (!load_playback_devices (list_store_p))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_playback_devices(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_TARGET_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
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

  std::string filename_string; // save-
#if defined(ACE_WIN32) || defined(ACE_WIN64)
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
  ACE_ASSERT (Common_File_Tools::isDirectory (voices_directory_string));
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_VOICE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERDIALOG_VOICE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  gboolean result = FALSE;
  result =
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                         voices_directory_string.c_str ());
  ACE_ASSERT (result);
  result =
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_dialog_p),
                                         voices_directory_string.c_str ());
  ACE_ASSERT (result);
  //if (unlikely (!result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to gtk_file_chooser_set_file(\"%s\"): \"%m\", aborting\n"),
  //              ACE_TEXT (filename_string.c_str ())));
  //  return G_SOURCE_REMOVE;
  //} // end IF

  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
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

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_ADAPTER_NAME)));
  ACE_ASSERT (list_store_p);
  if (!load_display_adapters (list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_display_adapters(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
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
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
  if (!load_displays (list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_displays(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
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

  GtkTextBuffer* text_buffer_p =
    GTK_TEXT_BUFFER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTBUFFER_NAME)));
  ACE_ASSERT (text_buffer_p);
  GtkTextIter text_iterator;
  gtk_text_buffer_get_iter_at_offset (text_buffer_p,
                                      &text_iterator,
                                      0); // offset
  GtkTextMark* text_mark_p =
    gtk_text_buffer_create_mark (text_buffer_p,
                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTMARK_BEGIN_NAME),
                                 &text_iterator,
                                 TRUE); // left gravity ?
  ACE_ASSERT (text_mark_p);
  gtk_text_mark_set_visible (text_mark_p,
                             TRUE); // visible

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
                               TRUE); // left gravity
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

  //--------------------------------------

  // step9: draw main dialog
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  // step11: activate some widgets
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_VOICE_NAME)));
  ACE_ASSERT (combo_box_p);
#if GTK_CHECK_VERSION(2, 30, 0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value,
                      voice_string.c_str ());
  Common_UI_GTK_Tools::selectValue (combo_box_p,
                                    value,
                                    0);
  g_value_unset (&value);

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAYBACK_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p, (mute_b ? FALSE : TRUE));

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_TARGET_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
  gtk_combo_box_set_active (combo_box_p, 0);

  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p, TRUE);
  if (filename_string.empty ())
    gtk_toggle_button_set_active (toggle_button_p, FALSE);

  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p, TRUE);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
  gtk_combo_box_set_active (combo_box_p, 0);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
  gtk_combo_box_set_active (combo_box_p, 0);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (!(*directshow_modulehandler_configuration_iterator).second.second->window);
      (*directshow_modulehandler_configuration_iterator).second.second->window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->window);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (!(*mediafoundation_modulehandler_configuration_iterator).second.second->window);
      (*mediafoundation_modulehandler_configuration_iterator).second.second->window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->window);
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
#endif // ACE_WIN32 || ACE_WIN64

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

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_VOICE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_TARGET_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);

  GtkTextView* text_view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (text_view_p);
  gtk_widget_set_sensitive (GTK_WIDGET (text_view_p), FALSE);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p = NULL;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      directshow_ui_cb_data_p->spectrumAnalyzerCBData.dispatch = NULL;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      mediafoundation_ui_cb_data_p->spectrumAnalyzerCBData.dispatch = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), continuing\n"),
                  ui_cb_data_p->mediaFramework));
      break;
    }
  } // end SWITCH
#else
  struct Test_I_ALSA_UI_CBData* alsa_ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (alsa_ui_cb_data_p);
  alsa_ui_cb_data_p->spectrumAnalyzerCBData.dispatch = NULL;
#endif // ACE_WIN32 || ACE_WIN64

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

  struct Test_I_CommandSpeech_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CommandSpeech_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
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
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
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
  struct Test_I_ALSA_UI_CBData* alsa_ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (alsa_ui_cb_data_p);
#endif // ACE_WIN32 || ACE_WIN64

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
                                     static_cast<gdouble> (ui_cb_data_p->progressData.statistic.bytes));

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

  //std::ostringstream converter;
  //converter << progress_data_p->statistic.messagesPerSecond;
  //converter << ACE_TEXT_ALWAYS_CHAR (" fps");
  //gtk_progress_bar_set_text (progress_bar_p,
  //                           (done ? ACE_TEXT_ALWAYS_CHAR ("")
  //                                 : converter.str ().c_str ()));
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
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GdkWindow* window_p = NULL;

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
                              FALSE); // invalidate children ?

continue_2:
  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
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
button_display_reset_clicked_cb (GtkWidget* widget_in,
                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_display_reset_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // button_display_reset_clicked_cb

void
button_display_settings_clicked_cb (GtkWidget* widget_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_display_settings_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // button_display_settings_clicked_cb

void
button_hw_settings_clicked_cb (GtkWidget* widget_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_hw_settings_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // button_hw_settings_clicked_cb

void
button_report_clicked_cb (GtkWidget* widget_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_report_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // button_report_clicked_cb

void
button_voice_reset_clicked_cb (GtkWidget* widget_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_voice_reset_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // button_voice_reset_clicked_cb

void
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p =
//    gtk_text_buffer_new (NULL); // text tag table --> create new
    gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_set_text (buffer_p,
                            ACE_TEXT_ALWAYS_CHAR (""), 0);
} // button_clear_clicked_cb

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
combobox_adapter_changed_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_adapter_changed_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // combobox_adapter_changed_cb

void
combobox_display_changed_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_display_changed_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // combobox_display_changed_cb

void
combobox_target_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_target_changed_cb"));

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
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_2; // renderer
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_2; // renderer
  std::string renderer_modulename_string;
  enum Stream_Device_Renderer renderer_e = STREAM_DEVICE_RENDERER_INVALID;
  HRESULT result = E_FAIL;
  IMFMediaSource* media_source_p = NULL;
  bool make_topology_b = false;
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

      renderer_e =
        directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer;
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
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      renderer_e =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->renderer;
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

  switch (renderer_e)
  {
    case STREAM_DEVICE_RENDERER_WAVEOUT:
    {
      renderer_modulename_string =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING);
      break;
    }
    case STREAM_DEVICE_RENDERER_WASAPI:
    {
      renderer_modulename_string =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING);
      break;
    }
    case STREAM_DEVICE_RENDERER_DIRECTSHOW:
    {
      renderer_modulename_string =
        ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING);
      break;
    }
    case STREAM_DEVICE_RENDERER_MEDIAFOUNDATION:
    {
      renderer_modulename_string =
        ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING);

      Test_I_MediaFoundation_Stream* stream_2 =
        dynamic_cast<Test_I_MediaFoundation_Stream*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (stream_2);
      Test_I_MediaFoundation_Target* target_p =
        &const_cast<Test_I_MediaFoundation_Target&> (stream_2->getR_4 ());
      if (!target_p->initialize (mediafoundation_ui_cb_data_p->configuration->mediaFoundationConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::initialize(), returning\n")));
        return;
      } // end IF
      media_source_p = target_p;
      make_topology_b = true;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown renderer type (was: %d), returning\n"),
                  renderer_e));
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_TARGET_NAME)));
  ACE_ASSERT (list_store_p);
  std::string device_identifier_string;
  gint card_id_i = std::numeric_limits<int>::max ();
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
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  card_id_i = g_value_get_int (&value);
  g_value_unset (&value);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  device_identifier_string = g_value_get_string (&value_2);
  g_value_unset (&value_2);

//  gint n_rows = 0;
  GtkToggleButton* toggle_button_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_modulehandler_configuration_iterator_2 =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (renderer_modulename_string);
      ACE_ASSERT (directshow_modulehandler_configuration_iterator_2 != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      Stream_Device_Identifier device_identifier;
      device_identifier.identifier._id = card_id_i;
      device_identifier.identifierDiscriminator = Stream_Device_Identifier::ID;
      (*directshow_modulehandler_configuration_iterator_2).second.second->deviceIdentifier =
        device_identifier;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_modulehandler_configuration_iterator_2 =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (renderer_modulename_string);
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator_2 != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      Stream_Device_Identifier device_identifier;
      device_identifier.identifier._guid =
        Common_OS_Tools::StringToGUID (device_identifier_string);
      device_identifier.identifierDiscriminator = Stream_Device_Identifier::GUID;
      (*mediafoundation_modulehandler_configuration_iterator_2).second.second->deviceIdentifier =
        device_identifier;
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

  GtkScale* hscale_p = NULL;
  std::ostringstream converter;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_DirectShow_Source* directshow_source_impl_2 = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  IMFSampleGrabberSinkCallback2* sample_grabber_p = NULL;
#else
  IMFSampleGrabberSinkCallback* sample_grabber_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  IMFTopology* topology_p = NULL;
 
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      IAMBufferNegotiation* buffer_negotiation_p = NULL;
      if (!Stream_Device_DirectShow_Tools::loadDeviceGraph ((*directshow_modulehandler_configuration_iterator_2).second.second->deviceIdentifier,
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
      if (!make_topology_b)
        break;
      
      ACE_ASSERT (media_source_p);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology ((*mediafoundation_modulehandler_configuration_iterator_2).second.second->deviceIdentifier,
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
//continue_:
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

  // get/set playback volume
  hscale_p =
        GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_HSCALE_VOLUME_NAME)));
  ACE_ASSERT (hscale_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // retrieve volume control handle
  // step1: retrieve DirectSound device GUID from wave device id
  struct _GUID GUID_s = Common_OS_Tools::StringToGUID (device_identifier_string);
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
      if (directshow_ui_cb_data_p->volumeControl)
      {
        directshow_ui_cb_data_p->volumeControl->Release (); directshow_ui_cb_data_p->volumeControl = NULL;
      } // end IF
      directshow_ui_cb_data_p->volumeControl = i_audio_endpoint_volume_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (mediafoundation_ui_cb_data_p->volumeControl)
      {
        mediafoundation_ui_cb_data_p->volumeControl->Release (); mediafoundation_ui_cb_data_p->volumeControl = NULL;
      } // end IF
      mediafoundation_ui_cb_data_p->volumeControl = i_audio_endpoint_volume_p;

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
      return;
    }
  } // end SWITCH
  gtk_range_set_value (GTK_RANGE (hscale_p),
                       static_cast<gdouble> (volume_level_f) * 100.0);
#else
  if (!Stream_MediaFramework_ALSA_Tools::getVolumeLevels (device_identifier_string,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_PLAYBACK_DEFAULT_SELEM_VOLUME_NAME),
                                                          false, // playback
                                                          min_level_i,
                                                          max_level_i,
                                                          current_level_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getVolumeLevels(\"%s\",\"%s\"), returning\n"),
                ACE_TEXT (device_identifier_string.c_str ()),
                ACE_TEXT (STREAM_LIB_ALSA_PLAYBACK_DEFAULT_SELEM_VOLUME_NAME)));
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
} // combobox_target_changed_cb

void
combobox_voice_changed_cb (GtkWidget* widget_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_voice_changed_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // combobox_voice_changed_cb

#if GTK_CHECK_VERSION (3,0,0)
gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  // sanity check(s)
  struct Test_I_CommandSpeech_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CommandSpeech_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ui_cb_data_base_p->spectrumAnalyzerCBData.context = context_in;
  ui_cb_data_base_p->spectrumAnalyzerCBData.window =
    gtk_widget_get_window (widget_in);
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.window)
    return FALSE; // not realized yet
  if (!ui_cb_data_base_p->spectrumAnalyzerCBData.dispatch)
    return FALSE; // stream not running (yet)
//  Stream_IStreamControlBase* stream_p = NULL;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p =
//    NULL;
//  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
//    NULL;
//  switch (ui_cb_data_base_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//      // sanity check(s)
//      directshow_ui_cb_data_p =
//        static_cast<struct Test_I_DirectShow_UI_CBData*> (userData_in);
//      ACE_ASSERT (directshow_ui_cb_data_p);
//      ACE_ASSERT (directshow_ui_cb_data_p->stream);
//      stream_p = directshow_ui_cb_data_p->stream;
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//      // sanity check(s)
//      mediafoundation_ui_cb_data_p =
//        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
//      ACE_ASSERT (mediafoundation_ui_cb_data_p);
//      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
//      stream_p = mediafoundation_ui_cb_data_p->stream;
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                  ui_cb_data_base_p->mediaFramework));
//      return FALSE;
//    }
//  } // end SWITCH
//#else
//  // sanity check(s)
//  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
//    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_p);
//  ACE_ASSERT (ui_cb_data_p->stream);
//  stream_p = ui_cb_data_p->stream;
//#endif // ACE_WIN32 || ACE_WIN64
//  ACE_ASSERT (stream_p);
//  if (!stream_p->isRunning ())
//    return FALSE; // stream not running (yet)

  try {
    ui_cb_data_base_p->spectrumAnalyzerCBData.dispatch->dispatch (&ui_cb_data_base_p->spectrumAnalyzerCBData);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  return TRUE;
} // drawingarea_draw_cb

void
drawingarea_size_allocate_cb (GtkWidget* widget_in,
                              GdkRectangle* allocation_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_size_allocate_cb"));

  ACE_UNUSED_ARG (allocation_in);

  // sanity check(s)
  struct Test_I_CommandSpeech_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CommandSpeech_UI_CBData*> (userData_in);
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
#else
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
  struct Test_I_CommandSpeech_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CommandSpeech_UI_CBData*> (userData_in);
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

gboolean
drawingarea_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey keyEvent_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_key_press_event_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
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
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return FALSE; // <-- propagate event
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
    { ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
      (*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode =
          (((*directshow_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode == STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM
                                                                                                                                                                                : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
      (*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode =
        (((*mediafoundation_modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode == STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM
                                                                                                                                                                                   : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return FALSE; // <-- propagate event
    }
  } // end SWITCH
#else
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration);
  (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode =
    (((*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode == STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM
                                                                                                                                                               : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE);
#endif // ACE_WIN32 || ACE_WIN64

  return TRUE; // <-- stop propagation
} // drawingarea_key_press_event_cb

void
drawingarea_realize_cb (GtkWidget* widget_in,
                        gpointer   userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_realize_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (userData_in);
  struct Test_I_CommandSpeech_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CommandSpeech_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (!ui_cb_data_base_p->spectrumAnalyzerCBData.context);
  ACE_ASSERT (!ui_cb_data_base_p->spectrumAnalyzerCBData.window);

  ui_cb_data_base_p->spectrumAnalyzerCBData.window =
    gtk_widget_get_window (widget_in);
  ACE_ASSERT (ui_cb_data_base_p->spectrumAnalyzerCBData.window);
  ui_cb_data_base_p->spectrumAnalyzerCBData.context =
    gdk_cairo_create (ui_cb_data_base_p->spectrumAnalyzerCBData.window);
  ACE_ASSERT (ui_cb_data_base_p->spectrumAnalyzerCBData.context);
  cairo_set_line_width (ui_cb_data_base_p->spectrumAnalyzerCBData.context, 1.0);
} // drawingarea_realize_cb

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
    (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
  is_signed_format =
      snd_pcm_format_signed ((*modulehandler_configuration_iterator).second.second->outputFormat.format);
  sample_size =
      (snd_pcm_format_width ((*modulehandler_configuration_iterator).second.second->outputFormat.format) / 8);
  channels =
    (*modulehandler_configuration_iterator).second.second->outputFormat.channels;
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

void
filechooserbutton_voice_file_set_cb (GtkFileChooserButton* button_in,
                                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_voice_file_set_cb"));

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
} // filechooserbutton_voice_file_set_cb

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
      ACE_ASSERT (directshow_ui_cb_data_p->volumeControl);
      HRESULT result =
        directshow_ui_cb_data_p->volumeControl->SetMasterVolumeLevelScalar (static_cast<float> (gtk_range_get_value (range_in) / 100.0),
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
      ACE_ASSERT (mediafoundation_ui_cb_data_p->volumeControl);
      HRESULT result =
        mediafoundation_ui_cb_data_p->volumeControl->SetMasterVolumeLevelScalar (static_cast<float> (gtk_range_get_value (range_in) / 100.0),
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
textview_key_press_event_cb (GtkWidget* widget_in,
                             GdkEventKey keyEvent_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::textview_key_press_event_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
  if ((keyEvent_in.type != GDK_KEY_PRESS) ||
      (keyEvent_in.keyval != GDK_KEY_Return))
    return FALSE; // <-- propagate event

  GtkTextBuffer* text_buffer_p =
    GTK_TEXT_BUFFER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTBUFFER_NAME)));
  ACE_ASSERT (text_buffer_p);
  GtkTextMark* text_mark_p =
    gtk_text_buffer_get_mark (text_buffer_p,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTMARK_BEGIN_NAME));
  ACE_ASSERT (text_mark_p);
  GtkTextIter text_iterator;
  gtk_text_buffer_get_iter_at_mark (text_buffer_p,
                                    &text_iterator,
                                    text_mark_p);
  GtkTextIter text_iterator_2;
  gtk_text_buffer_get_end_iter (text_buffer_p,
                                &text_iterator_2);
  gchar* text_p = gtk_text_buffer_get_text (text_buffer_p,
                                            &text_iterator,
                                            &text_iterator_2,
                                            TRUE); // include hidden chars
  ACE_ASSERT (text_p);
  std::string text_string = Common_UI_GTK_Tools::UTF8ToLocale (text_p, -1);
  g_free (text_p); text_p = NULL;
  gtk_text_buffer_insert (text_buffer_p,
                          &text_iterator_2,
                          keyEvent_in.string,
                          -1);
  gtk_text_buffer_get_end_iter (text_buffer_p,
                                &text_iterator_2);
  gtk_text_buffer_move_mark (text_buffer_p,
                             text_mark_p,
                             &text_iterator_2);

  // drop data into the processing stream
  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
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
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->messageAllocator);
      message_block_p =
          static_cast<ACE_Message_Block*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->messageAllocator->malloc (text_string.size () + 1));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->messageAllocator);
      message_block_p =
          static_cast<ACE_Message_Block*> (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->messageAllocator->malloc (text_string.size () + 1));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return TRUE;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->configuration->streamConfiguration.configuration_);
  ACE_ASSERT (ui_cb_data_p->configuration->streamConfiguration.configuration_->messageAllocator);
  message_block_p =
      static_cast<ACE_Message_Block*> (ui_cb_data_p->configuration->streamConfiguration.configuration_->messageAllocator->malloc (text_string.size () + 1));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (message_block_p);
  result = message_block_p->copy (text_string.c_str (),
                                  text_string.size ());
  ACE_ASSERT (result == 0);
  *message_block_p->wr_ptr () = 0; // 0-terminate string
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      Test_I_DirectShow_Stream* stream_p =
        dynamic_cast<Test_I_DirectShow_Stream*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (stream_p);
      result = stream_p->put (message_block_p, NULL);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      Test_I_MediaFoundation_Stream* stream_p =
        dynamic_cast<Test_I_MediaFoundation_Stream*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (stream_p);
      result = stream_p->put (message_block_p, NULL);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return TRUE;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->stream);
  Test_I_ALSA_Stream* stream_p =
    dynamic_cast<Test_I_ALSA_Stream*> (ui_cb_data_p->stream);
  ACE_ASSERT (stream_p);
  result = stream_p->put (message_block_p, NULL);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result != -1);

  return TRUE; // <-- do not propagate event
} // textview_key_press_event_cb

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

void
togglebutton_display_toggled_cb (GtkToggleButton* toggleButton_in,
                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_display_toggled_cb"));

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

  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
  GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_DISPLAY_2_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
  GtkButtonBox* button_box_p =
      GTK_BUTTON_BOX (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTONBOX_DISPLAY_NAME)));
  ACE_ASSERT (button_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
} // togglebutton_display_toggled_cb

void
togglebutton_fullscreen_toggled_cb (GtkToggleButton* toggleButton_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_fullscreen_toggled_cb"));

  // sanity check(s)
//  struct Test_I_UI_CBData* ui_cb_data_base_p =
//    static_cast<struct Test_I_UI_CBData*> (userData_in);
//  ACE_ASSERT (ui_cb_data_base_p);
//  ACE_ASSERT (ui_cb_data_base_p->UIState);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());
} // togglebutton_fullscreen_toggled_cb

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator_2; // file writer
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator_2; // file writer
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
      directshow_modulehandler_configuration_iterator_2 =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator_2 != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_);

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
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      mediafoundation_modulehandler_configuration_iterator_2 =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator_2 != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_);

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
  // sanity check(s)
  struct Test_I_ALSA_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_ALSA_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
  ACE_ASSERT (modulehandler_configuration_iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());

  stream_p = ui_cb_data_p->stream;
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

  struct Test_I_CommandSpeech_UI_ThreadData* thread_data_p = NULL;
  ACE_TCHAR thread_name[BUFSIZ];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  struct Test_I_CommandSpeech_UI_ProgressData* progress_data_p = NULL;

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  std::string filename_string;
  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SAVE_NAME)));
    ACE_ASSERT (entry_p);
    GtkFileChooserButton* file_chooser_button_p =
      GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
    ACE_ASSERT (file_chooser_button_p);
    gchar* text_2 =
      gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p));
    filename_string =
      Common_UI_GTK_Tools::UTF8ToLocale (ACE_TEXT_ALWAYS_CHAR (text_2));
    g_free (text_2); text_2 = NULL;
    filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
    filename_string +=
      Common_UI_GTK_Tools::UTF8ToLocale (ACE_TEXT_ALWAYS_CHAR (gtk_entry_get_text (entry_p)));
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator_2).second.second->fileIdentifier.identifier =
        filename_string;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator_2).second.second->fileIdentifier.identifier =
        filename_string;
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
  (*modulehandler_configuration_iterator_2).second.second->fileIdentifier.identifier =
    filename_string;
#endif // ACE_WIN32 || ACE_WIN64

  // step2: modify widgets
  gtk_button_set_label (GTK_BUTTON (toggleButton_in), GTK_STOCK_MEDIA_STOP);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_VOICE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_TARGET_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);

  GtkTextView* text_view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (text_view_p);
  gtk_widget_set_sensitive (GTK_WIDGET (text_view_p), TRUE);
  gtk_widget_grab_focus (GTK_WIDGET (text_view_p));

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
  ACE_OS::memset (&progress_data_p->statistic, 0, sizeof (struct Stream_Statistic));
  ACE_OS::memset (thread_name, 0, sizeof (ACE_TCHAR[BUFSIZ]));
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

    thread_data_p->infoEventSourceId =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                     idle_update_info_display_cb,
                     userData_in);
    if (thread_data_p->infoEventSourceId > 0)
      ui_cb_data_base_p->UIState->eventSourceIds.insert (thread_data_p->infoEventSourceId);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", returning\n")));
      return;
    } // end ELSE

    // step4: initialize UI updates
    thread_data_p->displayEventSourceId =
      g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_CAIRO_MS,
                     idle_update_display_cb,
                     userData_in);
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_display_cb,
      //                 userData_in,
      //                 NULL);
    if (thread_data_p->displayEventSourceId > 0)
      ui_cb_data_base_p->UIState->eventSourceIds.insert (thread_data_p->displayEventSourceId);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", returning\n")));
      return;
    } // end ELSE
  } // end lock scope
} // togglebutton_record_toggled_cb

void
togglebutton_playback_toggled_cb (GtkToggleButton* toggleButton_in,
                                  gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_save_toggled_cb"));

  // sanity check(s)
  struct Test_I_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    ui_cb_data_base_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != ui_cb_data_base_p->UIState->builders.end ());

  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_TARGET_2_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
} // togglebutton_playback_toggled_cb

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

  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_SAVE_2_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
} // togglebutton_save_toggled_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
