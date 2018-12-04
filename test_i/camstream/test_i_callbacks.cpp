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

#include "test_i_callbacks.h"

#include <limits>
#include <map>
#include <set>
#include <sstream>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <strmif.h>
#include <reftime.h>
#include <dvdmedia.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include "gdk/gdkwin32.h"
#else
#include "ace/Dirent_Selector.h"

//#include "gdk/gdkpixbuf.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Guard_T.h"
#include "ace/Synch.h"

#include "common.h"
#include "common_file_tools.h"
#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "stream_dev_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_file_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_i_camstream_defines.h"
#include "test_i_common_modules.h"
#include "test_i_target_message.h"
#include "test_i_source_common.h"
#include "test_i_target_common.h"
#include "test_i_target_listener_common.h"
#include "test_i_target_session_message.h"
#include "test_i_target_stream.h"

// initialize statics
static bool un_toggling_stream = false;

int
dirent_selector (const dirent* dirEntry_in)
{
  // *IMPORTANT NOTE*: select all files

  // sanity check --> ignore dot/double-dot
  if (ACE_OS::strncmp (dirEntry_in->d_name,
                       ACE_TEXT_ALWAYS_CHAR ("video"),
                       ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR ("video"))) != 0)
  {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("ignoring \"%s\"...\n"),
//                 ACE_TEXT (dirEntry_in->d_name)));
    return 0;
  } // end IF

  return 1;
}
int
dirent_comparator (const dirent** d1,
                   const dirent** d2)
{
  return ACE_OS::strcmp ((*d1)->d_name,
                         (*d2)->d_name);
}

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_capture_devices (enum Stream_MediaFramework_Type mediaFrameWork_in,
                      GtkListStore* listStore_in)
#else
load_capture_devices (GtkListStore* listStore_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::load_capture_devices"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  std::vector<std::pair<std::string, std::string> > listbox_entries_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  std::string friendly_name_string;

  switch (mediaFrameWork_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      ICreateDevEnum* enumerator_p = NULL;
      IEnumMoniker* enum_moniker_p = NULL;
      IMoniker* moniker_p = NULL;
      IPropertyBag* properties_p = NULL;
      struct tagVARIANT variant_s;

      result_2 =
        CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                          CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&enumerator_p));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, false).c_str ())));
        return false;
      } // end IF
      ACE_ASSERT (enumerator_p);

      result_2 =
        enumerator_p->CreateClassEnumerator (CLSID_VideoInputDeviceCategory,
                                             &enum_moniker_p,
                                             0);
      if (result_2 != S_OK)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(CLSID_VideoInputDeviceCategory): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, false).c_str ())));
        //result_2 = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        goto error;
      } // end IF
      ACE_ASSERT (enum_moniker_p);
      enumerator_p->Release (); enumerator_p = NULL;

      VariantInit (&variant_s);
      while (enum_moniker_p->Next (1, &moniker_p, NULL) == S_OK)
      { ACE_ASSERT (moniker_p);
        properties_p = NULL;
        result = moniker_p->BindToStorage (0, 0, IID_PPV_ARGS (&properties_p));
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
          goto error;
        } // end IF
        ACE_ASSERT (properties_p);

        result_2 =
          properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING,
                              &variant_s,
                              0);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                      ACE_TEXT (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
          goto error;
        } // end IF
        ACE_Wide_To_Ascii converter (variant_s.bstrVal);
        result_2 = VariantClear (&variant_s);
        ACE_ASSERT (SUCCEEDED (result_2));
        friendly_name_string = converter.char_rep ();

        result_2 =
          properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING,
                              &variant_s,
                              0);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                      ACE_TEXT (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
          goto error;
        } // end IF
        properties_p->Release (); properties_p = NULL;
        ACE_Wide_To_Ascii converter_2 (variant_s.bstrVal);
        result_2 = VariantClear (&variant_s);
        ACE_ASSERT (SUCCEEDED (result_2));

        listbox_entries_a.push_back (std::make_pair (friendly_name_string,
                                                     converter_2.char_rep ()));

        moniker_p->Release (); moniker_p = NULL;
      } // end WHILE
      enum_moniker_p->Release (); enum_moniker_p = NULL;

error:
      if (properties_p)
        properties_p->Release ();
      if (moniker_p)
        moniker_p->Release ();
      if (enum_moniker_p)
        enum_moniker_p->Release ();
      if (enumerator_p)
        enumerator_p->Release ();

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      IMFActivate** devices_pp = NULL;
      UINT32 count = 0;
      IMFAttributes* attributes_p = NULL;
      UINT32 length = 0;
      WCHAR buffer_a[BUFSIZ];

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
                               MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
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
#else
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error_2;
      } // end IF
      attributes_p->Release (); attributes_p = NULL;
      ACE_ASSERT (devices_pp);

      for (UINT32 index = 0; index < count; index++)
      {
        ACE_OS::memset (buffer_a, 0, sizeof (WCHAR[BUFSIZ]));
        length = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
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
        friendly_name_string =
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
        listbox_entries_a.push_back (std::make_pair (friendly_name_string,
                                                     ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a))));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
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

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFrameWork_in));
      return false;
    }
  } // end SWITCH

  result = true;
#else
  std::string directory (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY));
  ACE_Dirent_Selector entries;
  int result_2 = entries.open (ACE_TEXT (directory.c_str ()),
                               &dirent_selector,
                               &dirent_comparator);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Dirent_Selector::open(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (directory.c_str ())));
    return false;
  } // end IF
  if (entries.length () == 0)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no video capture devices found, continuing\n")));

  struct v4l2_capability device_capabilities;
  std::string device_filename;
  ACE_DIRENT* dirent_p = NULL;
  int file_descriptor = -1;
  int open_mode = O_RDONLY;
  for (unsigned int i = 0;
       i < static_cast<unsigned int> (entries.length ());
       ++i)
  {
    dirent_p = entries[i];
    ACE_ASSERT (dirent_p);

    device_filename = directory +
                      ACE_DIRECTORY_SEPARATOR_CHAR +
                      dirent_p->d_name;
    ACE_ASSERT (Common_File_Tools::isValidFilename (device_filename));
//    ACE_ASSERT (Common_File_Tools::isReadable (device_filename));

    file_descriptor = -1;
    file_descriptor = v4l2_open (device_filename.c_str (),
                                 open_mode);
    if (file_descriptor == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", aborting\n"),
                  ACE_TEXT (device_filename.c_str ()), open_mode));
      goto clean;
    } // end IF

    ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
    result_2 = v4l2_ioctl (file_descriptor,
                           VIDIOC_QUERYCAP,
                           &device_capabilities);
    if (result_2 == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%u): \"%m\", continuing\n"),
                  file_descriptor, ACE_TEXT ("VIDIOC_QUERYCAP")));
      goto close;
    } // end IF

    listbox_entries_a.push_back (std::make_pair (reinterpret_cast<char*> (device_capabilities.card),
                                                 device_filename));

close:
    result_2 = v4l2_close (file_descriptor);
    if (result_2 == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", aborting\n"),
                  file_descriptor));
      goto clean;
    } // end IF
  } // end FOR
  result = true;

clean:
  result_2 = entries.close ();
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Dirent_Selector::close(\"%s\"): \"%m\", continuing\n"),
                ACE_TEXT (directory.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  GtkTreeIter iterator;
  for (std::vector<std::pair<std::string, std::string> >::const_iterator iterator_2 = listbox_entries_a.begin ();
       iterator_2 != listbox_entries_a.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ACE_TEXT ((*iterator_2).first.c_str ()),
                        1, ACE_TEXT ((*iterator_2).second.c_str ()),
                        -1);
  } // end FOR

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct less_guid
{
  bool operator() (REFGUID lhs_in, REFGUID rhs_in) const
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
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
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
    if (!InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) &&
        !InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
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
                        0, Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str (),
                        1, Common_Tools::GUIDToString (*iterator_2).c_str (),
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
  struct _GUID GUID_s = GUID_NULL;
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

    GUIDs.insert (GUID_s);
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
  for (std::set<GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str (),
                        1, Common_Tools::GUIDToString (*iterator_2).c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
load_resolutions (IAMStreamConfig* IAMStreamConfig_in,
                  const struct _GUID& mediaSubType_in,
                  GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_resolutions"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  int count = 0, size = 0;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  std::set<std::pair<unsigned int, unsigned int> > resolutions;
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
    if (!InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      resolutions.insert (std::make_pair (video_info_header_p->bmiHeader.biWidth,
                                          video_info_header_p->bmiHeader.biHeight));
    } // end IF
    else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    {
      // *NOTE*: these media subtypes do not work with the Video Renderer
      //         directly --> insert the Overlay Mixer
      video_info_header2_p = (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      resolutions.insert (std::make_pair (video_info_header_p->bmiHeader.biWidth,
                                          video_info_header_p->bmiHeader.biHeight));
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown video format type, aborting\n")));
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      return false;
    } // end ELSE
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  } // end WHILE

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = resolutions.begin ();
       iterator_2 != resolutions.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator_2).first;
    converter << 'x';
    converter << (*iterator_2).second;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, (*iterator_2).first,
                        2, (*iterator_2).second,
                        -1);
  } // end FOR

  return true;
}
bool
//load_resolutions (IMFSourceReader* IMFSourceReader_in,
load_resolutions (IMFMediaSource* IMFMediaSource_in,
                  const struct _GUID& mediaSubType_in,
                  GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_resolutions"));

  // sanity check(s)
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (IMFMediaSource_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  std::set<std::pair<unsigned int, unsigned int> > resolutions;
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
  UINT32 width, height;
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
      result = MFGetAttributeSize (media_type_p,
                                   MF_MT_FRAME_SIZE,
                                   &width, &height);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        media_type_p->Release (); media_type_p = NULL;
        return false;
      } // end IF
      resolutions.insert (std::make_pair (width, height));
    } // end IF
    media_type_p->Release (); media_type_p = NULL;
    ++count;
  } // end WHILE
  media_type_handler_p->Release ();
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
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = resolutions.begin ();
       iterator_2 != resolutions.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator_2).first;
    converter << 'x';
    converter << (*iterator_2).second;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, (*iterator_2).first,
                        2, (*iterator_2).second,
                        -1);
  } // end FOR

  return true;
}

bool
load_rates (IAMStreamConfig* IAMStreamConfig_in,
            const struct _GUID& mediaSubType_in,
            unsigned int width_in,
            GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_rates"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = S_OK;
  int count = 0, size = 0;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  unsigned int frame_duration;
  std::set<std::pair<unsigned int, unsigned int> > frame_rates;
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
    if (InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      if (video_info_header_p->bmiHeader.biWidth != width_in)
      {
        Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
        continue;
      } // end IF
      else
        frame_duration =
          static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end IF
    else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    {
      video_info_header2_p =
        (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      if (video_info_header2_p->bmiHeader.biWidth != width_in)
      {
        Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
        continue;
      } // end IF
      else
        frame_duration =
          static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end ELSEIF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid AM_MEDIA_TYPE, aborting\n")));
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      return false;
    } // end IF
    frame_rates.insert (std::make_pair ((10000000 / frame_duration), 1));
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = frame_rates.begin ();
       iterator_2 != frame_rates.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).first,
                        1, (*iterator_2).second,
                        -1);
  } // end FOR

  return true;
}
bool
//load_rates (IMFSourceReader* IMFSourceReader_in,
load_rates (IMFMediaSource* IMFMediaSource_in,
            const struct _GUID& mediaSubType_in,
            unsigned int width_in,
            GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_rates"));

  // sanity check(s)
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (IMFMediaSource_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = S_OK;
  std::set<std::pair<unsigned int, unsigned int> > frame_rates;
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
  UINT32 width, height;
  UINT32 numerator, denominator;
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
    result = MFGetAttributeSize (media_type_p,
                                 MF_MT_FRAME_SIZE,
                                 &width, &height);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      media_type_p->Release (); media_type_p = NULL;
      return false;
    } // end IF

    if (InlineIsEqualGUID (GUID_s, mediaSubType_in) &&
        (width == width_in))
    {
      result = MFGetAttributeRatio (media_type_p,
                                    MF_MT_FRAME_RATE,
                                    &numerator, &denominator);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        media_type_p->Release (); media_type_p = NULL;
        return false;
      } // end IF
      frame_rates.insert (std::make_pair (numerator, denominator));
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
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = frame_rates.begin ();
       iterator_2 != frame_rates.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).first,
                        1, (*iterator_2).second,
                        -1);
  } // end FOR

  return true;
}
#else
bool
load_formats (int fd_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  int result = -1;
  std::map<__u32, std::string> formats;
  struct v4l2_fmtdesc format_description;
  ACE_OS::memset (&format_description, 0, sizeof (struct v4l2_fmtdesc));
  format_description.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  do
  {
    result = v4l2_ioctl (fd_in,
                         VIDIOC_ENUM_FMT,
                         &format_description);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EINVAL)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                    fd_in, ACE_TEXT ("VIDIOC_ENUM_FMT")));
      break;
    } // end IF
    ++format_description.index;

    formats.insert (std::make_pair (format_description.pixelformat,
                                    reinterpret_cast<char*> (format_description.description)));
  } while (true);

  std::ostringstream converter;
  GtkTreeIter iterator;
  for (std::map<__u32, std::string>::const_iterator iterator_2 = formats.begin ();
       iterator_2 != formats.end ();
       ++iterator_2)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << (*iterator_2).first;

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).second.c_str (),
                        1, converter.str ().c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
load_resolutions (int fd_in,
                  __u32 format_in,
                  GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_resolutions"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  int result = -1;
  std::set<std::pair<unsigned int, unsigned int> > resolutions;
  struct v4l2_frmsizeenum resolution_description;
  ACE_OS::memset (&resolution_description, 0, sizeof (struct v4l2_frmsizeenum));
  resolution_description.pixel_format = format_in;
  do
  {
    result = v4l2_ioctl (fd_in,
                         VIDIOC_ENUM_FRAMESIZES,
                         &resolution_description);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EINVAL)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                    fd_in, ACE_TEXT ("VIDIOC_ENUM_FRAMESIZES")));
      break;
    } // end IF
    ++resolution_description.index;

    if (resolution_description.type != V4L2_FRMSIZE_TYPE_DISCRETE)
      continue;

    resolutions.insert (std::make_pair (resolution_description.discrete.width,
                                        resolution_description.discrete.height));
  } while (true);

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = resolutions.begin ();
       iterator_2 != resolutions.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (*iterator_2).first;
    converter << 'x';
    converter << (*iterator_2).second;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, (*iterator_2).first,
                        2, (*iterator_2).second,
                        -1);
  } // end FOR

  return true;
}

struct less_fract
{
  bool operator() (const struct v4l2_fract& lhs_in,
                   const struct v4l2_fract& rhs_in) const
  {
    return ((lhs_in.numerator / lhs_in.denominator) <
            (rhs_in.numerator / rhs_in.denominator));
  }
};
bool
load_rates (int fd_in,
            __u32 format_in,
            unsigned int width_in, unsigned int height_in,
            GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_rates"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  int result = -1;
  std::set<struct v4l2_fract, less_fract> frame_intervals;
  struct v4l2_frmivalenum frame_interval_description;
  ACE_OS::memset (&frame_interval_description,
                  0,
                  sizeof (struct v4l2_frmivalenum));
  frame_interval_description.pixel_format = format_in;
  frame_interval_description.width = width_in;
  frame_interval_description.height = height_in;
  do
  {
    result = v4l2_ioctl (fd_in,
                         VIDIOC_ENUM_FRAMEINTERVALS,
                         &frame_interval_description);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EINVAL)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                    fd_in, ACE_TEXT ("VIDIOC_ENUM_FRAMEINTERVALS")));
        return false;
      } // end IF
      break; // done
    } // end IF

    if (frame_interval_description.index == 0)
    {
      switch (frame_interval_description.type)
      {
        case V4L2_FRMIVAL_TYPE_DISCRETE:
          break;
        case V4L2_FRMIVAL_TYPE_CONTINUOUS:
        case V4L2_FRMIVAL_TYPE_STEPWISE:
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("the device supports 'non-discrete' frame rates (min/max/step: %u/%u/%u); this is currently not supported\n"),
                      frame_interval_description.stepwise.min.denominator, frame_interval_description.stepwise.max.denominator,
                      frame_interval_description.stepwise.step));

          frame_intervals.insert (frame_interval_description.stepwise.max);

          goto continue_;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown frame interval type (was: %d), aborting\n"),
                      frame_interval_description.type));
          return false;
        }
      } // end SWITCH
    } // end IF
    ACE_ASSERT (frame_interval_description.type ==
                V4L2_FRMIVAL_TYPE_DISCRETE);
    ++frame_interval_description.index;

    frame_intervals.insert (frame_interval_description.discrete);
  } while (true);

continue_:
  GtkTreeIter iterator;
  guint frame_rate = 0;
  for (std::set<v4l2_fract, less_fract>::const_iterator iterator_2 = frame_intervals.begin ();
       iterator_2 != frame_intervals.end ();
       ++iterator_2)
  {
    frame_rate =
        (((*iterator_2).numerator == 1) ? (*iterator_2).denominator
                                        : static_cast<guint> (static_cast<float> ((*iterator_2).denominator) /
                                                              static_cast<float> ((*iterator_2).numerator)));
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, frame_rate,
                        1, (*iterator_2).numerator,
                        2, (*iterator_2).denominator,
                        -1);
  } // end FOR

  return true;
}
#endif

//////////////////////////////////////////

void
set_capture_format (struct Test_I_CamStream_UI_CBData* CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::set_capture_format"));

  // sanity check(s)
  ACE_ASSERT (CBData_in);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR_2 ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (CBData_in);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (CBData_in);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  CBData_in->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (CBData_in);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_HOLDS (&value, G_TYPE_STRING));
  std::string format_string = g_value_get_string (&value);
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID media_subtype = Common_Tools::StringToGUID (format_string);
  ACE_ASSERT (!InlineIsEqualGUID (media_subtype, GUID_NULL));
#endif
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
    return; // <-- nothing selected
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  g_value_init (&value_2, G_TYPE_UINT);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  unsigned int width = g_value_get_uint (&value);
  g_value_unset (&value);
  unsigned int height = g_value_get_uint (&value_2);
  g_value_unset (&value_2);
  unsigned int framerate_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.builder);

      // step1: set capture format
      if ((*directshow_modulehandler_iterator).second.second.sourceFormat)
        Stream_MediaFramework_DirectShow_Tools::delete_ ((*directshow_modulehandler_iterator).second.second.sourceFormat);
      if (!Stream_Device_DirectShow_Tools::getVideoCaptureFormat ((*directshow_modulehandler_iterator).second.second.builder,
                                                                  media_subtype,
                                                                  width, height,
                                                                  framerate_i,
                                                                  (*directshow_modulehandler_iterator).second.second.sourceFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::getVideoCaptureFormat(%s,%ux%u@%ufps), returning\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                    width, height,
                    framerate_i));
        return;
      } // end IF
      ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.sourceFormat);
      if (!Stream_Device_DirectShow_Tools::setCaptureFormat ((*directshow_modulehandler_iterator).second.second.builder,
                                                             CLSID_VideoInputDeviceCategory,
                                                             *(*directshow_modulehandler_iterator).second.second.sourceFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::setCaptureFormat(), returning\n")));
        return;
      } // end IF

      // step2: adjust output format
      // sanity check(s)
      ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.inputFormat);
      if (InlineIsEqualGUID ((*directshow_modulehandler_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo))
      { ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.inputFormat->cbFormat == sizeof (struct tagVIDEOINFOHEADER));
        struct tagVIDEOINFOHEADER* video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> ((*directshow_modulehandler_iterator).second.second.inputFormat->pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight = height;
        video_info_header_p->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_p->bmiHeader);
        ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
        video_info_header_p->dwBitRate =
          (video_info_header_p->bmiHeader.biSizeImage * 8) *                      // bits / frame
          (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

        (*directshow_modulehandler_iterator).second.second.inputFormat->lSampleSize =
          video_info_header_p->bmiHeader.biSizeImage;
      } // end IF
      else if (InlineIsEqualGUID ((*directshow_modulehandler_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo2))
      { ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.inputFormat->cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
        struct tagVIDEOINFOHEADER2* video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> ((*directshow_modulehandler_iterator).second.second.inputFormat->pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight = height;
        video_info_header_p->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_p->bmiHeader);
        ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
        video_info_header_p->dwBitRate =
          (video_info_header_p->bmiHeader.biSizeImage * 8) *                      // bits / frame
          (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

        (*directshow_modulehandler_iterator).second.second.inputFormat->lSampleSize =
          video_info_header_p->bmiHeader.biSizeImage;
      } // end IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString ((*directshow_modulehandler_iterator).second.second.inputFormat->formattype).c_str ())));
        return;
      } // end ELSE

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  CBData_in->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  ACE_ASSERT (false); // *TODO*
  ACE_UNUSED_ARG (height);
  ACE_UNUSED_ARG (width);
#endif
}

void
update_buffer_size (struct Test_I_CamStream_UI_CBData* CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::update_buffer_size"));

  // sanity check(s)
  ACE_ASSERT (CBData_in);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR_2 ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (CBData_in);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (CBData_in);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  CBData_in->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (CBData_in);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  unsigned int frame_size_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.sourceFormat);
      frame_size_i = 
        Stream_MediaFramework_Tools::frameSize (*(*directshow_modulehandler_iterator).second.second.sourceFormat);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.sourceFormat);
      struct _GUID media_subtype = GUID_NULL;
      HRESULT result =
        (*mediafoundation_modulehandler_iterator).second.second.sourceFormat->GetGUID (MF_MT_SUBTYPE,
                                                                                       &media_subtype);
      ACE_ASSERT (SUCCEEDED (result));
      UINT32 width, height;
      result =
        MFGetAttributeSize ((*mediafoundation_modulehandler_iterator).second.second.sourceFormat,
                            MF_MT_FRAME_SIZE,
                            &width, &height);
      result = MFCalculateImageSize (media_subtype,
                                     width, height,
                                     &frame_size_i);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFCalculateImageSize(\"%s\",%u,%u): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_subtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
                    width, height,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  CBData_in->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  frame_size_i =
    (*modulehandler_iterator).second.second.sourceFormat.format.sizeimage;
#endif // ACE_WIN32 || ACE_WIN64
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<gdouble> (frame_size_i));
}

//////////////////////////////////////////

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  struct Test_I_CamStream_ThreadData* thread_data_p =
    static_cast<struct Test_I_CamStream_ThreadData*> (arg_in);
  ACE_ASSERT (thread_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator;
  ACE_SYNCH_MUTEX* lock_p = NULL;
  enum Net_TransportLayerType protocol = NET_TRANSPORTLAYER_INVALID;
  Stream_IStreamControlBase* stream_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  std::ostringstream converter;
  const Stream_SessionData* session_ui_cb_data_p = NULL;
  bool result_2 = false;
  guint context_id = 0;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_ThreadData* directshow_thread_data_p = NULL;
  struct Test_I_Source_MediaFoundation_ThreadData* mediafoundation_thread_data_p =
    NULL;
  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (thread_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_thread_data_p =
        static_cast<struct Test_I_Source_DirectShow_ThreadData*> (arg_in);
      ACE_ASSERT (directshow_thread_data_p->CBData);
      ACE_ASSERT (directshow_thread_data_p->CBData->configuration);

      iterator =
        state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      lock_p = &state_r.lock;
      protocol = directshow_thread_data_p->CBData->configuration->protocol;

      directshow_stream_iterator =
        directshow_thread_data_p->CBData->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_thread_data_p->CBData->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_thread_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_ThreadData*> (arg_in);
      ACE_ASSERT (mediafoundation_thread_data_p->CBData);
      ACE_ASSERT (mediafoundation_thread_data_p->CBData->configuration);

      iterator =
        state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      lock_p = &state_r.lock;
      protocol = mediafoundation_thread_data_p->CBData->configuration->protocol;

      mediafoundation_stream_iterator =
        mediafoundation_thread_data_p->CBData->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_thread_data_p->CBData->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  thread_data_p->mediaFramework));
      goto done;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_ThreadData* v4l2_thread_data_p =
    static_cast<struct Test_I_Source_V4L2_ThreadData*> (arg_in);
  // sanity check(s)
  ACE_ASSERT (v4l2_thread_data_p->CBData);
  ACE_ASSERT (v4l2_thread_data_p->CBData->configuration);

  iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  lock_p = &state_r.lock;
  protocol = v4l2_thread_data_p->CBData->configuration->protocol;
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_thread_data_p->CBData->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_thread_data_p->CBData->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (lock_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *lock_p, -1);
#else
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *lock_p, std::numeric_limits<void*>::max ());
#endif // ACE_WIN32 || ACE_WIN64
    // retrieve stream handle
    switch (protocol)
    {
      case NET_TRANSPORTLAYER_TCP:
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (thread_data_p->mediaFramework)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            stream_p = directshow_thread_data_p->CBData->stream;
            //(*directshow_modulehandler_iterator).second.second.stream =
            //  directshow_ui_cb_data_p->CBData->stream;
            result_2 =
              directshow_thread_data_p->CBData->stream->initialize ((*directshow_stream_iterator).second);
            const Test_I_Source_DirectShow_SessionData_t* session_data_container_p =
              &directshow_thread_data_p->CBData->stream->getR ();
            session_ui_cb_data_p =
              &const_cast<struct Test_I_Source_DirectShow_SessionData&> (session_data_container_p->getR ());
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            stream_p = mediafoundation_thread_data_p->CBData->stream;
            //(*mediafoundation_modulehandler_iterator).second.second.stream =
            //  mediafoundation_ui_cb_data_p->CBData->stream;
            result_2 =
              mediafoundation_thread_data_p->CBData->stream->initialize ((*mediafoundation_stream_iterator).second);
            const Test_I_Source_MediaFoundation_SessionData_t* session_data_container_p =
              &mediafoundation_thread_data_p->CBData->stream->getR ();
            session_ui_cb_data_p =
              &const_cast<struct Test_I_Source_MediaFoundation_SessionData&> (session_data_container_p->getR ());
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                        thread_data_p->mediaFramework));
            goto done;
          }
        } // end SWITCH
#else
        stream_p = v4l2_thread_data_p->CBData->stream;
        //(*iterator_2).second.second.stream = ui_cb_data_p->CBData->stream;
        result_2 =
          v4l2_thread_data_p->CBData->stream->initialize ((*stream_iterator).second);
        const Test_I_Source_V4L2_SessionData_t* session_data_container_p =
          &v4l2_thread_data_p->CBData->stream->getR ();
        session_ui_cb_data_p =
          &const_cast<struct Test_I_Source_V4L2_SessionData&> (session_data_container_p->getR ());
#endif // ACE_WIN32 || ACE_WIN64
        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (thread_data_p->mediaFramework)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            stream_p = directshow_thread_data_p->CBData->UDPStream;
            //(*directshow_modulehandler_iterator).second.stream =
            //  directshow_ui_cb_data_p->CBData->UDPStream;
            result_2 =
              directshow_thread_data_p->CBData->UDPStream->initialize ((*directshow_stream_iterator).second);
            const Test_I_Source_DirectShow_SessionData_t* session_data_container_p =
              &directshow_thread_data_p->CBData->UDPStream->getR ();
            session_ui_cb_data_p =
              &const_cast<struct Test_I_Source_DirectShow_SessionData&> (session_data_container_p->getR ());
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            stream_p = mediafoundation_thread_data_p->CBData->UDPStream;
            //(*mediafoundation_modulehandler_iterator).second.stream =
            //  mediafoundation_ui_cb_data_p->CBData->UDPStream;
            result_2 =
              mediafoundation_thread_data_p->CBData->UDPStream->initialize ((*mediafoundation_stream_iterator).second);
            const Test_I_Source_MediaFoundation_SessionData_t* session_data_container_p =
              &mediafoundation_thread_data_p->CBData->UDPStream->getR ();
            session_ui_cb_data_p =
              &const_cast<struct Test_I_Source_MediaFoundation_SessionData&> (session_data_container_p->getR ());
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                        thread_data_p->mediaFramework));
            goto done;
          }
        } // end SWITCH
#else
        stream_p = v4l2_thread_data_p->CBData->UDPStream;
        //(*iterator_2).second.stream = ui_cb_data_p->CBData->UDPStream;
        result_2 =
          v4l2_thread_data_p->CBData->UDPStream->initialize ((*stream_iterator).second);
        const Test_I_Source_V4L2_SessionData_t* session_data_container_p =
          &v4l2_thread_data_p->CBData->UDPStream->getR ();
        session_ui_cb_data_p =
          &const_cast<struct Test_I_Source_V4L2_SessionData&> (session_data_container_p->getR ());
#endif // ACE_WIN32 || ACE_WIN64
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                    thread_data_p->CBData->configuration->protocol));
        goto done;
      }
    } // end SWITCH
    ACE_ASSERT (session_ui_cb_data_p);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << session_ui_cb_data_p->sessionId;

    // retrieve status bar handle
    gdk_threads_enter ();
    statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_STATUSBAR_NAME)));
    ACE_ASSERT (statusbar_p);
    context_id = gtk_statusbar_get_context_id (statusbar_p,
                                               converter.str ().c_str ());
    gdk_threads_leave ();

    // set context ID
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (thread_data_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        (*directshow_modulehandler_iterator).second.second.contextId =
          context_id;
        break;
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        (*mediafoundation_modulehandler_iterator).second.second.contextId =
          context_id;
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    thread_data_p->mediaFramework));
        goto done;
      }
    } // end SWITCH
#else
    (*modulehandler_iterator).second.second.contextId = context_id;
#endif // ACE_WIN32 || ACE_WIN64
  } // end lock scope
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream: \"%m\", aborting\n")));
    goto done;
  } // end IF
  ACE_ASSERT (stream_p);

  // *NOTE*: processing currently happens 'inline' (borrows calling thread)
  stream_p->start ();
  //    if (!stream_p->isRunning ())
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to start stream, aborting\n")));
  //      return;
  //    } // end IF
  stream_p->wait (true, false, false);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif // ACE_WIN32 || ACE_WIN64

done:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *lock_p, -1);
#else
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *lock_p, std::numeric_limits<void*>::max ());
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (thread_data_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        directshow_thread_data_p->CBData->progressData.completedActions.insert (thread_data_p->eventSourceId);
        break;
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        mediafoundation_thread_data_p->CBData->progressData.completedActions.insert (thread_data_p->eventSourceId);
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    thread_data_p->mediaFramework));
        goto done;
      }
    } // end SWITCH
  } // end lock scope
#else
    v4l2_thread_data_p->CBData->progressData.completedActions.insert (thread_data_p->eventSourceId);
  } // end lock scope
#endif // ACE_WIN32 || ACE_WIN64

  // clean up
  delete thread_data_p; thread_data_p = NULL;

  return result;
}

//////////////////////////////////////////

gboolean
idle_initialize_source_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_source_UI_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());
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
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (v4l2_ui_cb_data_p->configuration);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT hresult = CoInitializeEx (NULL,
                                    COINIT_MULTITHREADED);
  if (FAILED (hresult))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (hresult).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#endif

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
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
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_capture_devices (ui_cb_data_base_p->mediaFramework,
                             list_store_p))
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
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RATE_NAME)));
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

  GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ENTRY_DESTINATION_NAME)));
  ACE_ASSERT (entry_p);
  const char* string_p = NULL;
  u_short port_number = 0;
  enum Net_TransportLayerType protocol = NET_TRANSPORTLAYER_INVALID;
  bool use_reactor = false;
  bool use_loopback = false;
  unsigned int buffer_size = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Test_I_Source_DirectShow_ConnectionConfigurationIterator_t iterator_2 =
        directshow_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator_2 != directshow_ui_cb_data_p->configuration->connectionConfigurations.end ());
      string_p =
        (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.address.get_host_name ();
      port_number =
        (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ();
      protocol = directshow_ui_cb_data_p->configuration->protocol;
      use_reactor =
        (directshow_ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
      use_loopback =
        (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice;
      buffer_size =
        (*directshow_stream_iterator).second.allocatorConfiguration_.defaultBufferSize;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Test_I_Source_MediaFoundation_ConnectionConfigurationIterator_t iterator_2 =
      mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator_2 != mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.end ());
      string_p =
        (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.address.get_host_name ();
      port_number =
        (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ();
      protocol = mediafoundation_ui_cb_data_p->configuration->protocol;
      use_reactor =
        (mediafoundation_ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
      use_loopback =
        (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice;
      buffer_size =
        (*mediafoundation_stream_iterator).second.allocatorConfiguration_.defaultBufferSize;
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
  Test_I_Source_V4L2_ConnectionConfigurationIterator_t iterator_2 =
    v4l2_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != v4l2_ui_cb_data_p->configuration->connectionConfigurations.end ());
  string_p =
    (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.address.get_host_name ();
  port_number =
    (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ();
  protocol = v4l2_ui_cb_data_p->configuration->protocol;
  use_reactor =
          (v4l2_ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
  use_loopback =
    (*iterator_2).second.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice;
  buffer_size =
    (*stream_iterator).second.allocatorConfiguration_.defaultBufferSize;
#endif
  ACE_ASSERT (string_p);
  gtk_entry_set_text (entry_p, string_p);

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                              static_cast<double> (port_number));

  GtkRadioButton* radio_button_p = NULL;
  if (protocol == NET_TRANSPORTLAYER_UDP)
  {
    radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME)));
    ACE_ASSERT (radio_button_p);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p), TRUE);
  } // end IF
  GtkCheckButton* check_button_p =
      GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                !use_reactor);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                use_loopback);

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (buffer_size));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
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
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT),
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
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  // step5: initialize updates
  //Test_I_GTK_CBData* cb_ui_cb_data_p = ui_cb_data_p;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    guint event_source_id = g_timeout_add_seconds (1,
                                                   idle_update_log_display_cb,
                                                   ui_cb_data_base_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
                     idle_update_info_display_cb,
                     ui_cb_data_base_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?

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

  // step6b: connect custom signals
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);

  GObject* object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (toggleaction_stream_toggled_cb),
                               userData_in);
  ACE_ASSERT (result_2);
//  object_p =
//    gtk_builder_get_object ((*iterator).second.second,
//                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME));
//  ACE_ASSERT (object_p);
//  result_2 =
//    g_signal_connect (object_p,
//                      ACE_TEXT_ALWAYS_CHAR ("activate"),
//                      G_CALLBACK (action_settings_activate_cb),
//                      userData_in);
//  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_RESET_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_reset_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_source_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("value-changed"),
  //                             G_CALLBACK (spinbutton_port_value_changed_cb),
  //                             cb_ui_cb_data_p);
  //ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_format_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_resolution_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RATE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_rate_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_ui_cb_data_p);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_ui_cb_data_p);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                      G_CALLBACK (textview_size_allocate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //-------------------------------------

#if GTK_CHECK_VERSION (3,0,0)
  result_2 =
      g_signal_connect (G_OBJECT (drawing_area_p),
                        ACE_TEXT_ALWAYS_CHAR ("draw"),
                        G_CALLBACK (drawingarea_draw_cb),
                        userData_in);
#else
  result_2 =
      g_signal_connect (G_OBJECT (drawing_area_p),
                        ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                        G_CALLBACK (drawingarea_draw_cb),
                        userData_in);
#endif
  ACE_ASSERT (result_2);
#if GTK_CHECK_VERSION (3,0,0)
  result_2 =
      g_signal_connect (G_OBJECT (drawing_area_p),
                        ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                        G_CALLBACK (drawingarea_size_allocate_source_cb),
                        userData_in);
#else
  result_2 =
    g_signal_connect (G_OBJECT (drawing_area_p),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (drawingarea_configure_event_source_cb),
                      userData_in);
#endif
  ACE_ASSERT (result_2);

  //-------------------------------------

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLEAR_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                      G_CALLBACK (button_clear_clicked_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step10: retrieve canvas coordinates, window handle and pixel buffer
  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT (window_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      ACE_ASSERT (!(*directshow_modulehandler_iterator).second.second.window);
      (*directshow_modulehandler_iterator).second.second.window =
        gdk_win32_window_get_impl_hwnd (window_p);
      //static_cast<HWND> (GDK_WINDOW_HWND (GDK_DRAWABLE (window_p)));
      ACE_ASSERT (IsWindow ((*directshow_modulehandler_iterator).second.second.window));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("drawing area window handle: 0x%@\n"),
                  (*directshow_modulehandler_iterator).second.second.window));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ACE_ASSERT (!(*mediafoundation_modulehandler_iterator).second.second.window);
      (*mediafoundation_modulehandler_iterator).second.second.window =
        gdk_win32_window_get_impl_hwnd (window_p);
      //static_cast<HWND> (GDK_WINDOW_HWND (GDK_DRAWABLE (window_p)));
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
  Test_I_Source_V4L2_StreamConfigurationsIterator_t iterator_3 =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator_3 != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T iterator_4 =
      (*iterator_3).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_4 != (*iterator_3).second.end ());
  ACE_ASSERT (!(*iterator_4).second.second.window);
  (*iterator_4).second.second.window = window_p;
#endif
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (allocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_iterator).second.second.area.bottom =
        allocation.height;
      (*directshow_modulehandler_iterator).second.second.area.left =
        allocation.x;
      (*directshow_modulehandler_iterator).second.second.area.right =
        allocation.width;
      (*directshow_modulehandler_iterator).second.second.area.top =
        allocation.y;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_iterator).second.second.area.bottom =
      allocation.height;
      (*mediafoundation_modulehandler_iterator).second.second.area.left =
        allocation.x;
      (*mediafoundation_modulehandler_iterator).second.second.area.right =
        allocation.width;
      (*mediafoundation_modulehandler_iterator).second.second.area.top =
        allocation.y;
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
  (*iterator_4).second.second.area = allocation;

  ACE_ASSERT (!v4l2_ui_cb_data_p->pixelBuffer);
  v4l2_ui_cb_data_p->pixelBuffer =
#if GTK_CHECK_VERSION (3,0,0)
      gdk_pixbuf_get_from_window ((*iterator_4).second.second.window,
                                  0, 0,
                                  allocation.width, allocation.height);
#else
      gdk_pixbuf_get_from_drawable (NULL,
                                    GDK_DRAWABLE ((*iterator_4).second.second.window),
                                    NULL,
                                    0, 0,
                                    0, 0, allocation.width, allocation.height);
#endif
  if (!v4l2_ui_cb_data_p->pixelBuffer)
  { // *NOTE*: most probable reason: window is not mapped
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  (*iterator_4).second.second.pixelBuffer = v4l2_ui_cb_data_p->pixelBuffer;
#endif

  // step11: select default capture source (if any)
  //         --> populate the option-comboboxes
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
  else
  {
    GtkToggleAction* toggle_action_p =
      GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME)));
    ACE_ASSERT (toggle_action_p);
    gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), false);
  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_source_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_end_source_UI_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // *IMPORTANT NOTE*: there are two major reasons for being here that are not
  //                   mutually exclusive, so there could be a race:
  //                   - user pressed stop
  //                   - there was an asynchronous error on the stream
  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_stock_id (GTK_ACTION (toggle_action_p), GTK_STOCK_MEDIA_PLAY);
  if (gtk_toggle_action_get_active (toggle_action_p))
  {
    un_toggling_stream = true;
    gtk_action_activate (GTK_ACTION (toggle_action_p));
  } // end IF

  GtkAction* action_p =
//    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME)));
//  ACE_ASSERT (action_p);
//  gtk_action_set_sensitive (action_p, true);
//  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_RESET_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  // stop progress reporting
  GtkProgressBar* progress_bar_p = NULL;
  if (!ui_cb_data_p->progressData.eventSourceId)
    goto continue_;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard_2, state_r.lock, G_SOURCE_REMOVE);
    if (!g_source_remove (ui_cb_data_p->progressData.eventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  ui_cb_data_p->progressData.eventSourceId));
    state_r.eventSourceIds.erase (ui_cb_data_p->progressData.eventSourceId);
    ui_cb_data_p->progressData.eventSourceId = 0;

    ACE_OS::memset (&(ui_cb_data_p->progressData.statistic),
                    0,
                    sizeof (ui_cb_data_p->progressData.statistic));
  } // end lock scope
  progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), FALSE);

continue_:
  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_source_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_source_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_ProgressData* progress_data_p =
    static_cast<struct Test_I_CamStream_UI_ProgressData*> (userData_in);
  ACE_ASSERT (progress_data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // synch access
//  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  int result = -1;
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: join completed thread(s)
  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = progress_data_p->completedActions.begin ();
       iterator_3 != progress_data_p->completedActions.end ();
       ++iterator_3)
  {
    iterator_2 = progress_data_p->pendingActions.find (*iterator_3);
    ACE_ASSERT (iterator_2 != progress_data_p->pendingActions.end ());
    ACE_thread_t thread_id = (*iterator_2).second.id ();
    result = thread_manager_p->join (thread_id, &exit_status);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%u): \"%m\", continuing\n"),
                  thread_id));
    else if (exit_status)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("thread %u has joined (status was: %d)...\n"),
                  thread_id,
                  exit_status));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("thread %u has joined (status was: %@)...\n"),
                  thread_id,
                  exit_status));
#endif
    } // end IF

    state_r.eventSourceIds.erase (*iterator_3);
    progress_data_p->pendingActions.erase (iterator_2);
  } // end FOR
  progress_data_p->completedActions.clear ();

  bool done = false;
  if (progress_data_p->pendingActions.empty ())
  {
    //if (ui_cb_data_p->cursorType != GDK_LAST_CURSOR)
    //{
    //  GdkCursor* cursor_p = gdk_cursor_new (ui_cb_data_p->cursorType);
    //  if (!cursor_p)
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to gdk_cursor_new(%d): \"%m\", continuing\n"),
    //                ui_cb_data_p->cursorType));
    //    return FALSE; // G_SOURCE_REMOVE
    //  } // end IF
    //  GtkWindow* window_p =
    //    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
    //                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
    //  ACE_ASSERT (window_p);
    //  GdkWindow* window_2 = gtk_widget_get_window (GTK_WIDGET (window_p));
    //  ACE_ASSERT (window_2);
    //  gdk_window_set_cursor (window_2, cursor_p);
    //  ui_cb_data_p->cursorType = GDK_LAST_CURSOR;
    //} // end IF

    done = true;
  } // end IF

  // step2: update progress bar text
  std::ostringstream converter;
  //{ ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard_2, progress_data_p->state->lock, G_SOURCE_REMOVE);
    converter << progress_data_p->statistic.messagesPerSecond;
  //} // end lock scope
  std::string progressbar_text = converter.str ();
  progressbar_text += ACE_TEXT_ALWAYS_CHAR (" fps");
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : ACE_TEXT_ALWAYS_CHAR (progressbar_text.c_str ())));
  if (done)
  {
    gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);
  } // end IF
  else
    gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

//////////////////////////////////////////

gboolean
idle_initialize_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_target_UI_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);

  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Target_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Target_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_Target_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Target_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Target_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Target_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
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
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_I_Target_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  //Common_UI_GladeXMLsIterator_t iterator =
  //  ui_cb_data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != ui_cb_data_p->gladeXML.end ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT hresult = CoInitializeEx (NULL,
                                    (COINIT_MULTITHREADED    |
                                     COINIT_DISABLE_OLE1DDE  |
                                     COINIT_SPEED_OVER_MEMORY));
  if (FAILED (hresult)) // RPC_E_CHANGED_MODE : 0x80010106L
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (hresult).c_str ())));
#endif

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
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
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GFile* file_p = NULL;
  std::string directory, file_name;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directory =
        (*directshow_modulehandler_iterator).second.second.targetFileName;
      file_name =
        (*directshow_modulehandler_iterator).second.second.targetFileName;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      directory =
        (*mediafoundation_modulehandler_iterator).second.second.targetFileName;
      file_name =
        (*mediafoundation_modulehandler_iterator).second.second.targetFileName;
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
  Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator_2 =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());

  directory = (*iterator_2).second.second.targetFileName;
  file_name = (*iterator_2).second.second.targetFileName;
#endif
  // sanity check(s)
  if (!Common_File_Tools::isDirectory (directory))
  {
    directory =
      ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
    if (Common_File_Tools::isValidPath (directory))
    {
      if (!Common_File_Tools::isDirectory (directory))
        if (!Common_File_Tools::createDirectory (directory))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to create directory \"%s\": \"%m\", aborting\n"),
                      ACE_TEXT (directory.c_str ())));
          return G_SOURCE_REMOVE;
        } // end IF
    } // end IF
    else if (Common_File_Tools::isValidFilename (directory))
    {
      directory =
        ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
      if (!Common_File_Tools::isDirectory (directory))
        if (!Common_File_Tools::createDirectory (directory))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to create directory \"%s\": \"%m\", aborting\n"),
                      ACE_TEXT (directory.c_str ())));
          return G_SOURCE_REMOVE;
        } // end IF
    } // end IF
    else
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid target directory (was: \"%s\"), falling back\n"),
                  ACE_TEXT (directory.c_str ())));
      directory = Common_File_Tools::getTempDirectory ();
    } // end ELSE
  } // end IF
  if (Common_File_Tools::isDirectory (file_name))
    file_name =
      ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILENAME);
  else if (Common_File_Tools::isValidFilename (file_name))
    file_name =
      ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
  file_name = directory +
              ACE_DIRECTORY_SEPARATOR_CHAR_A +
              file_name;
  ACE_ASSERT (Common_File_Tools::isValidFilename (file_name));
  file_p = g_file_new_for_path (file_name.c_str ());
  ACE_ASSERT (file_p);
  //GFile* file_2 = g_file_get_parent (file_p);
  //if (!file_2)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_file_get_parent(), aborting\n")));

  //  // clean up
  //  g_object_unref (file_p);

  //  return G_SOURCE_REMOVE;
  //} // end IF
  //g_object_unref (file_p);
  //char* string_p = g_file_get_path (file_2);
  char* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(), aborting\n")));

    // clean up
    //g_object_unref (file_2);
    g_object_unref (file_p);

    return G_SOURCE_REMOVE;
  } // end IF
  //g_object_unref (file_2);
  g_object_unref (file_p);
  //if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
  if (!gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                         string_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_filename(), aborting\n")));

    // clean up
    g_free (string_p);

    return G_SOURCE_REMOVE;
  } // end IF
  g_free (string_p);

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  unsigned short port_number = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Target_MediaFoundation_ConnectionConfigurationIterator_t mediafoundation_connection_configuration_iterator;
  Test_I_Target_DirectShow_ConnectionConfigurationIterator_t directshow_connection_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_connection_configuration_iterator =
        directshow_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_connection_configuration_iterator != directshow_ui_cb_data_p->configuration->connectionConfigurations.end ());
      port_number =
        (*directshow_connection_configuration_iterator).second.socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_connection_configuration_iterator =
      mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_connection_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.end ());
      port_number =
        (*mediafoundation_connection_configuration_iterator).second.socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ();
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
  Test_I_Target_ConnectionConfigurationIterator_t iterator_3 =
    ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->connectionConfigurations.end ());
  port_number =
    (*iterator_3).second.socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ();
#endif
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (port_number));

  GtkRadioButton* radio_button_p = NULL;
  enum Net_TransportLayerType protocol = NET_TRANSPORTLAYER_INVALID;
  bool use_reactor =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      protocol = directshow_ui_cb_data_p->configuration->protocol;
      use_reactor =
        (directshow_ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      protocol = mediafoundation_ui_cb_data_p->configuration->protocol;
      use_reactor =
        (mediafoundation_ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
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
  protocol = ui_cb_data_p->configuration->protocol;
  use_reactor =
    (ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
#endif
  if (protocol == NET_TRANSPORTLAYER_UDP)
  {
    radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME)));
    ACE_ASSERT (radio_button_p);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p), TRUE);
  } // end IF
  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                !use_reactor);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  bool use_loopback = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      use_loopback =
        (*directshow_connection_configuration_iterator).second.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      use_loopback =
        (*mediafoundation_connection_configuration_iterator).second.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  use_loopback =
    (*iterator_3).second.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice;
#endif
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                use_loopback);

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  unsigned int buffer_size = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      buffer_size =
        directshow_ui_cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      buffer_size =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  buffer_size =
    ui_cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize;
#endif
  gtk_spin_button_set_value (spin_button_p,
                              static_cast<double> (buffer_size));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
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
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT),
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
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  // step5: initialize updates
  guint event_source_id = 0;
  //Test_I_GTK_CBData* cb_ui_cb_data_p = ui_cb_data_base_p;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    event_source_id = g_timeout_add_seconds (1,
                                             idle_update_log_display_cb,
                                             ui_cb_data_base_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id = g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
                                     idle_update_info_display_cb,
                                     ui_cb_data_base_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: (auto-)connect signals/slots
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
                      G_CALLBACK (filechooserbutton_target_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (GTK_FILE_CHOOSER (file_chooser_button_p),
                      //ACE_TEXT_ALWAYS_CHAR ("current-folder-changed"),
                      ACE_TEXT_ALWAYS_CHAR ("selection-changed"),
                      G_CALLBACK (filechooser_target_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  result_2 =
    g_signal_connect (file_chooser_dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("file-activated"),
                      G_CALLBACK (filechooserdialog_target_cb),
                      NULL);
  ACE_ASSERT (result_2);

  // step6b: connect custom signals
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);

  GObject* object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("toggled"),
                      G_CALLBACK (toggleaction_listen_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (action_close_all_activate_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_REPORT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_report_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //object_p =
  //    gtk_builder_get_object ((*iterator).second.second,
  //                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                           ACE_TEXT_ALWAYS_CHAR ("value-changed"),
  //                           G_CALLBACK (spinbutton_port_value_changed_cb),
  //                           cb_ui_cb_data_p);
  //ACE_ASSERT (result_2);

  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_ui_cb_data_p);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (togglebutton_protocol_toggled_cb),
  //                             cb_ui_cb_data_p);

  //  object_p =
  //    gtk_builder_get_object ((*iterator).second.second,
  //                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME));
  //  ACE_ASSERT (object_p);
  //  result_2 =
  //    g_signal_connect (object_p,
  //                      ACE_TEXT_ALWAYS_CHAR ("changed"),
  //                      G_CALLBACK (combobox_format_changed_cb),
  //                      userData_in);
  //  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR(TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                      G_CALLBACK (textview_size_allocate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //--------------------------------------

#if GTK_CHECK_VERSION (3,0,0)
  result_2 =
    g_signal_connect (G_OBJECT (drawing_area_p),
                      ACE_TEXT_ALWAYS_CHAR ("draw"),
                      G_CALLBACK (drawingarea_draw_cb),
                      userData_in);
#else
  result_2 =
    g_signal_connect (G_OBJECT (drawing_area_p),
                      ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                      G_CALLBACK (drawingarea_draw_cb),
                      userData_in);
#endif
  ACE_ASSERT (result_2);
#if GTK_CHECK_VERSION (3,0,0)
  result_2 =
      g_signal_connect (G_OBJECT (drawing_area_p),
                        ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                        G_CALLBACK (drawingarea_size_allocate_target_cb),
                        userData_in);
#else
  result_2 =
    g_signal_connect (G_OBJECT (drawing_area_p),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (drawingarea_configure_event_target_cb),
                      userData_in);
#endif
  ACE_ASSERT (result_2);

  //--------------------------------------

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_CLEAR_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                      G_CALLBACK (button_clear_clicked_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

  // step7: set defaults
  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_activate (GTK_ACTION (toggle_action_p));

  //file_chooser_button_p =
  //  GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      default_folder_uri +=
        (*directshow_modulehandler_iterator).second.second.targetFileName;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      default_folder_uri +=
        (*mediafoundation_modulehandler_iterator).second.second.targetFileName;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  default_folder_uri += (*iterator_2).second.second.targetFileName;
#endif
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
  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT (window_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (!(*directshow_modulehandler_iterator).second.second.window);
      // *TODO*: find out why the DirectShow video renderers do not draw onto
      //         GtkDrawingAreas (e.g. missing overlay function ?)
      (*directshow_modulehandler_iterator).second.second.window =
        gdk_win32_window_get_impl_hwnd (window_p);
      //static_cast<HWND> (GDK_WINDOW_HWND (GDK_DRAWABLE (window_p)));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (!(*mediafoundation_modulehandler_iterator).second.second.window);
      (*mediafoundation_modulehandler_iterator).second.second.window =
        gdk_win32_window_get_impl_hwnd (window_p);
      //static_cast<HWND> (GDK_WINDOW_HWND (GDK_DRAWABLE (window_p)));
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
  ACE_ASSERT (!(*iterator_2).second.second.window);
  (*iterator_2).second.second.window = window_p;
#endif
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (allocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
  g_signal_emit_by_name (G_OBJECT (drawing_area_p),
                         ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                         &allocation);
  GdkPixbuf* pixbuf_p = NULL;
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
  pixbuf_p =
    gdk_pixbuf_get_from_window (window_p,
                                0, 0,
                                allocation.width, allocation.height);
#else
  pixbuf_p =
    gdk_pixbuf_get_from_drawable (NULL,
                                  GDK_DRAWABLE (window_p),
                                  NULL,
                                  0, 0,
                                  0, 0, allocation.width, allocation.height);
#endif
  if (!pixbuf_p)
  { // *NOTE*: most probable reason: window is not mapped
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (!directshow_ui_cb_data_p->pixelBuffer);
      directshow_ui_cb_data_p->pixelBuffer = pixbuf_p;
      (*directshow_modulehandler_iterator).second.second.pixelBuffer = pixbuf_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (!mediafoundation_ui_cb_data_p->pixelBuffer);
      mediafoundation_ui_cb_data_p->pixelBuffer = pixbuf_p;
      (*mediafoundation_modulehandler_iterator).second.second.pixelBuffer =
        pixbuf_p;
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
  ACE_ASSERT (!(*iterator_2).second.second.pixelBuffer);
  (*iterator_2).second.second.pixelBuffer = pixbuf_p;
  ui_cb_data_p->pixelBuffer = pixbuf_p;
#endif

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_start_target_UI_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_end_target_UI_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  Test_I_Target_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
//  Test_I_Target_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p = NULL;
//  if (ui_cb_data_base_p->useMediaFoundation)
//  {
//    mediafoundation_ui_cb_data_p =
//      static_cast<Test_I_Target_MediaFoundation_UI_CBData*> (userData_in);
//
//    // sanity check(s)
//    ACE_ASSERT (mediafoundation_ui_cb_data_p);
//    ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
//  } // end IF
//  else
//  {
//    directshow_ui_cb_data_p =
//      static_cast<Test_I_Target_DirectShow_UI_CBData*> (userData_in);
//
//    // sanity check(s)
//    ACE_ASSERT (directshow_ui_cb_data_p);
//    ACE_ASSERT (directshow_ui_cb_data_p->configuration);
//  } // end ELSE
//#else
//  Test_I_Target_UI_CBData* ui_cb_data_p =
//    static_cast<Test_I_Target_UI_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);
//  ACE_ASSERT (ui_cb_data_p->configuration);
//#endif

  unsigned int connection_count = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: this doesn't always work as conceived. Depending on the runtime
  //         (and scheduling), the (last) connection may not yet have
  //         deregistered with the manager at this stage
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Test_I_Target_DirectShow_InetConnectionManager_t* connection_manager_p =
        TEST_I_TARGET_DIRECTSHOW_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (connection_manager_p);
      connection_count = connection_manager_p->count ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Test_I_Target_MediaFoundation_InetConnectionManager_t* connection_manager_p =
        TEST_I_TARGET_MEDIAFOUNDATION_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (connection_manager_p);
      connection_count = connection_manager_p->count ();
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
  Test_I_Target_InetConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_count = connection_manager_p->count ();
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p,
                            (connection_count != 0));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  if (connection_count == 0)
  {
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
      ACE_OS::memset (&(ui_cb_data_base_p->progressData.statistic),
                      0,
                      sizeof (ui_cb_data_base_p->progressData.statistic));
    } // end lock scope

    gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  } // end IF

  return G_SOURCE_REMOVE;
} // idle_end_target_UI_cb

gboolean
idle_reset_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_reset_target_UI_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    ui_cb_data_p->progressData.transferred = 0;
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_target_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_target_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_ProgressData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_ProgressData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;
  float fps, speed = 0.0F;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    fps   = ui_cb_data_p->statistic.messagesPerSecond;
    speed = ui_cb_data_p->statistic.bytesPerSecond;
  } // end lock scope
  std::string magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  if (speed)
  {
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("kbyte(s)/s");
    } // end IF
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("mbyte(s)/s");
    } // end IF
    result = ACE_OS::sprintf (buffer, ACE_TEXT ("%.0f fps | %.2f %s"),
                              fps, speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (buffer));
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

gboolean
idle_finalize_source_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_source_UI_cb"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_UNUSED_ARG (userData_in);
#else
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);

  // clean up
  int result = -1;
  if (v4l2_ui_cb_data_p->fileDescriptor != -1)
  {
    result = v4l2_close (v4l2_ui_cb_data_p->fileDescriptor);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  v4l2_ui_cb_data_p->fileDescriptor));
    v4l2_ui_cb_data_p->fileDescriptor = -1;
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
idle_finalize_target_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_target_UI_cb"));

  ACE_UNUSED_ARG (userData_in);

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
      static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p = NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<Test_I_Source_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
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
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (v4l2_ui_cb_data_p->configuration);
#endif
  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
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
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          if (spin_button_p) // target ?
            gtk_spin_button_spin (spin_button_p,
                                  GTK_SPIN_STEP_FORWARD,
                                  1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (ui_cb_data_base_p->progressData.transferred));

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_STOPPED:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          if (spin_button_p) // target ?
            gtk_spin_button_spin (spin_button_p,
                                  GTK_SPIN_STEP_BACKWARD,
                                  1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

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
idle_update_log_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.logStackLock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  GtkTextIter text_iterator;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iterator);

  gchar* string_p = NULL;
  // sanity check
  if (state_r.logStack.empty ())
    return G_SOURCE_CONTINUE;

  // step1: convert text
  while (!state_r.logStack.empty ())
  {
    string_p = Common_UI_GTK_Tools::localeToUTF8 (state_r.logStack.front ());
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"), aborting\n"),
                  ACE_TEXT (state_r.logStack.front ().c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF

    // step2: display text
    gtk_text_buffer_insert (buffer_p,
                            &text_iterator,
                            string_p,
                            -1);

    // clean up
    g_free (string_p); string_p = NULL;

    state_r.logStack.pop_front ();
  } // end WHILE

  state_r.logStack.clear ();

  // step3: scroll the view accordingly
//  // move the iterator to the beginning of line, so it doesn't scroll
//  // in horizontal direction
//  gtk_text_iter_set_line_offset (&text_iterator, 0);

//  // ...and place the mark at iter. The mark will stay there after insertion
//  // because it has "right" gravity
//  GtkTextMark* text_mark_p =
//      gtk_text_buffer_get_mark (buffer_p,
//                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SCROLLMARK_NAME));
////  gtk_text_buffer_move_mark (buffer_p,
////                             text_mark_p,
////                             &text_iterator);

//  // scroll the mark onscreen
//  gtk_text_view_scroll_mark_onscreen (view_p,
//                                      text_mark_p);
  //GtkAdjustment* adjustment_p =
  //    GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ADJUSTMENT_NAME)));
  //ACE_ASSERT (adjustment_p);
  //gtk_adjustment_set_value (adjustment_p,
  //                          adjustment_p->upper - adjustment_p->page_size));

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_video_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_video_display_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
      static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DRAWINGAREA_NAME)));
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
toggleaction_stream_toggled_cb (GtkToggleAction* toggleAction_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_stream_toggled_cb"));

  // handle untoggle --> PLAY
  if (un_toggling_stream)
  {
    un_toggling_stream = false;
    return; // done
  } // end IF

  // --> user pressed play/pause/stop

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
      NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);
      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);
      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);
  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (iterator != state_r.builders.end ());

  struct Test_I_CamStream_ThreadData* thread_data_p = NULL;
  enum Net_TransportLayerType protocol = NET_TRANSPORTLAYER_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      protocol = directshow_ui_cb_data_p->configuration->protocol;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      protocol = mediafoundation_ui_cb_data_p->configuration->protocol;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_ThreadData* thread_data_2 = NULL;
  ACE_thread_t thread_id = -1;
  protocol = v4l2_ui_cb_data_p->configuration->protocol;
#endif
  ACE_hthread_t thread_handle;
  ACE_TCHAR thread_name[BUFSIZ];
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;
  int result = -1;

  Stream_IStreamControlBase* stream_p = NULL;
  switch (protocol)
  {
    case NET_TRANSPORTLAYER_TCP:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (ui_cb_data_p->mediaFramework)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          stream_p = directshow_ui_cb_data_p->stream;
          break;
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          stream_p = mediafoundation_ui_cb_data_p->stream;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      ui_cb_data_p->mediaFramework));
          return;
      }
    } // end SWITCH
#else
      stream_p = v4l2_ui_cb_data_p->stream;
#endif
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (ui_cb_data_p->mediaFramework)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          stream_p = directshow_ui_cb_data_p->UDPStream;
          break;
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          stream_p = mediafoundation_ui_cb_data_p->UDPStream;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      ui_cb_data_p->mediaFramework));
          return;
        }
      } // end SWITCH
#else
      stream_p = v4l2_ui_cb_data_p->UDPStream;
#endif
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                  ui_cb_data_p->configuration->protocol));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (stream_p);

  GtkAction* action_p = NULL;
  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);

  // toggle ?
  if (!gtk_toggle_action_get_active (toggleAction_in))
  {
    // --> user pressed pause/stop

    //// step0: modify widgets
    //gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_PLAY);

    //action_p =
    //  //    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
    //  //                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME)));
    //  //  ACE_ASSERT (action_p);
    //  //  gtk_action_set_sensitive (action_p, true);
    //  //  action_p =
    //  GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
    //                                      ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_RESET_NAME)));
    //ACE_ASSERT (action_p);
    //gtk_action_set_sensitive (action_p, true);

    //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

    // step1: stop stream
    stream_p->stop (false, // wait ?
                    true); // locked access ?

    return;
  } // end IF

  // --> user pressed play

  if (ui_cb_data_p->isFirst)
    ui_cb_data_p->isFirst = false;

  // step0: modify widgets
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in),
                           GTK_STOCK_MEDIA_STOP);
  action_p =
//    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_SETTINGS_NAME)));
//  ACE_ASSERT (action_p);
//  gtk_action_set_sensitive (action_p, false);
//  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_RESET_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, false);

  GtkSpinButton* spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

  // step1: set up progress reporting
  ui_cb_data_p->progressData.transferred = 0;
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), TRUE);

  // step2: update configuration
  // retrieve source
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_4;
  if (gtk_combo_box_get_active_iter (combo_box_p,
                                     &iterator_4))
  {
    GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
    ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_4,
                              1, &value);
    ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        ACE_OS::strcpy ((*directshow_modulehandler_iterator).second.second.deviceIdentifier.identifier._string,
                        g_value_get_string (&value));
        break;
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        ACE_OS::strcpy ((*mediafoundation_modulehandler_iterator).second.second.deviceIdentifier.identifier._string,
                        g_value_get_string (&value));
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    ui_cb_data_p->mediaFramework));
        return;
      }
    } // end SWITCH
#else
    (*modulehandler_iterator).second.second.deviceIdentifier.identifier =
        g_value_get_string (&value);
#endif
    g_value_unset (&value);
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (v4l2_ui_cb_data_p->fileDescriptor != -1)
    (*modulehandler_iterator).second.second.fileDescriptor =
      v4l2_ui_cb_data_p->fileDescriptor;
#endif

  // retrieve port number
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  unsigned short port_number =
    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (spin_button_p));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Test_I_Source_DirectShow_ConnectionConfigurationIterator_t iterator_3 =
        directshow_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator_3 != directshow_ui_cb_data_p->configuration->connectionConfigurations.end ());
        (*iterator_3).second.socketHandlerConfiguration.socketConfiguration_2.address.set_port_number (port_number,
                                                                                                       1);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Test_I_Source_MediaFoundation_ConnectionConfigurationIterator_t iterator_3 =
        mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator_3 != mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.end ());
        (*iterator_3).second.socketHandlerConfiguration.socketConfiguration_2.address.set_port_number (port_number,
                                                                                                       1);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  Test_I_Source_V4L2_ConnectionConfigurationIterator_t iterator_5 =
    v4l2_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_5 != v4l2_ui_cb_data_p->configuration->connectionConfigurations.end ());
  (*iterator_5).second.socketHandlerConfiguration.socketConfiguration_2.address.set_port_number (port_number,
                                                                                                 1);
#endif

  // retrieve protocol
  GtkRadioButton* radio_button_p =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME)));
  ACE_ASSERT (radio_button_p);
  protocol =
    (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)) ? NET_TRANSPORTLAYER_TCP
                                                                       : NET_TRANSPORTLAYER_UDP);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      directshow_ui_cb_data_p->configuration->protocol = protocol;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      mediafoundation_ui_cb_data_p->configuration->protocol = protocol;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  v4l2_ui_cb_data_p->configuration->protocol = protocol;
#endif

  // retrieve buffer
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      (*directshow_stream_iterator).second.allocatorConfiguration_.defaultBufferSize =
        static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      (*mediafoundation_stream_iterator).second.allocatorConfiguration_.defaultBufferSize =
        static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*stream_iterator).second.allocatorConfiguration_.defaultBufferSize =
    static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));
#endif

  // sanity check(s)
  bool result_3 = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  IMFTopology* topology_p = NULL;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { 
      //ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.builder);
      //ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.inputFormat);
      result_3 = true;
      //  Stream_Device_DirectShow_Tools::setCaptureFormat ((*directshow_modulehandler_iterator).second.second.builder,
      //                                                           CLSID_VideoInputDeviceCategory,
      //                                                           *(*directshow_modulehandler_iterator).second.second.inputFormat);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.inputFormat);
      enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
        MFSESSION_GETFULLTOPOLOGY_CURRENT;
      if ((*mediafoundation_modulehandler_iterator).second.second.session)
      {
        result_2 =
          (*mediafoundation_modulehandler_iterator).second.second.session->GetFullTopology (flags,
                                                                                            0,
                                                                                            &topology_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error;
        } // end IF
      } // end IF
      ACE_ASSERT (topology_p);
      result_3 =
        Stream_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                               (*mediafoundation_modulehandler_iterator).second.second.inputFormat);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  result_3 =
    Stream_Device_Tools::setFormat ((*modulehandler_iterator).second.second.fileDescriptor,
                                    (*modulehandler_iterator).second.second.sourceFormat);
#endif
  if (!result_3)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::setFormat(), aborting\n")));
    goto error;
  } // end IF

  // step3: start processing thread
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      struct Test_I_Source_DirectShow_ThreadData* thread_data_2 = NULL;
      ACE_NEW_NORETURN (thread_data_2,
                        struct Test_I_Source_DirectShow_ThreadData ());
      if (thread_data_2)
      {
        thread_data_2->CBData = directshow_ui_cb_data_p;
        thread_data_p = thread_data_2;
      } // end ELSE
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      topology_p->Release ();
      //struct _AMMediaType* media_type_p = NULL;
      //Stream_Device_Tools::getCaptureFormat (ui_cb_data_p->configuration->moduleHandlerConfiguration.builder,
      //                                              media_type_p);
      //media_type.Set (*media_type_p);
      //ACE_ASSERT (media_type == *ui_cb_data_p->configuration->moduleHandlerConfiguration.format);

      struct Test_I_Source_MediaFoundation_ThreadData* thread_data_2 = NULL;
      ACE_NEW_NORETURN (thread_data_2,
                        struct Test_I_Source_MediaFoundation_ThreadData ());
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
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
  if (thread_data_p)
    thread_data_p->mediaFramework = ui_cb_data_p->mediaFramework;
#else
  ACE_NEW_NORETURN (thread_data_2,
                    struct Test_I_Source_V4L2_ThreadData ());
  if (thread_data_2)
  {
    thread_data_2->CBData = v4l2_ui_cb_data_p;
    thread_data_p = thread_data_2;
  } // end ELSE
#endif
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
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
  //                  ACE_TEXT (TEST_I_STREAM_FILECOPY_THREAD_NAME));
  //  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_I_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    ACE_THR_FUNC function_p = ACE_THR_FUNC (::stream_processing_function);
    result =
      thread_manager_p->spawn (function_p,                       // function
                               thread_data_p,                    // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),              // flags
                               &thread_id,                       // id
                               &thread_handle,                   // handle
                               ACE_DEFAULT_THREAD_PRIORITY,      // priority
                               COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1, // *TODO*: group id
                               NULL,                             // stack
                               0,                                // stack size
                               &thread_name_2);                  // name
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));

      // clean up
      delete thread_data_p;

      goto error;
    } // end IF

    // step3: start progress reporting
    ACE_ASSERT (!ui_cb_data_p->progressData.eventSourceId);
    ui_cb_data_p->progressData.eventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &ui_cb_data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                          COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
                          idle_update_progress_source_cb,
                          &ui_cb_data_p->progressData,
                          NULL);
    if (!ui_cb_data_p->progressData.eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_source_cb): \"%m\", returning\n")));

      // clean up
      ACE_THR_FUNC_RETURN exit_status;
      result = thread_manager_p->join (thread_id, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%u): \"%m\", continuing\n"),
                    thread_id));

      goto error;
    } // end IF
    thread_data_p->eventSourceId = ui_cb_data_p->progressData.eventSourceId;
    ui_cb_data_p->progressData.pendingActions[ui_cb_data_p->progressData.eventSourceId] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    state_r.eventSourceIds.insert (ui_cb_data_p->progressData.eventSourceId);
  } // end lock scope

  return;

error:
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_PLAY);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
  gtk_action_set_sensitive (action_p, true);
} // toggle_action_stream_toggled_cb

void
action_reset_activate_cb (GtkAction* action_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_reset_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
} // action_reset_activate_cb

void
action_settings_activate_cb (GtkAction* action_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_settings_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
} // action_settings_activate_cb

//////////////////////////////////////////

void
action_close_all_activate_cb (GtkAction* action_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_close_all_activate_cb"));

  // sanity check(s)
  ACE_ASSERT (action_in);
  gtk_action_set_sensitive (action_in, FALSE);
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Target_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Target_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Target_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
      static_cast<struct Test_I_Target_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
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
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->configuration);
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  int result = -1;

  // *PORTABILITY*: on MS Windows systems, user signals SIGUSRx are not defined
  //                --> use SIGBREAK (21) and SIGTERM (15) instead
  int signal = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signal = SIGTERM;
#else
  signal = SIGUSR2;
#endif
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // closed the UDP "listener" ? --> toggle listen button
  enum Net_TransportLayerType protocol = NET_TRANSPORTLAYER_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      protocol = directshow_ui_cb_data_p->configuration->protocol;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      protocol = mediafoundation_ui_cb_data_p->configuration->protocol;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  protocol = ui_cb_data_p->configuration->protocol;
#endif
  if (protocol == NET_TRANSPORTLAYER_UDP)
  {
    GtkToggleAction* toggle_action_p =
      GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME)));
    ACE_ASSERT (toggle_action_p);
    gtk_action_activate (GTK_ACTION (toggle_action_p));
  } // end IF
} // action_close_all_activate_cb

void
toggleaction_listen_activate_cb (GtkToggleAction* toggleAction_in,
                                 gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_listen_activate_cb"));

  ACE_UNUSED_ARG (toggleAction_in);

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Target_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Target_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_Target_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Target_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  Test_I_Target_DirectShow_ConnectionConfigurationIterator_t directshow_connection_configuration_iterator;
  Test_I_Target_MediaFoundation_ConnectionConfigurationIterator_t mediafoundation_connection_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Target_DirectShow_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      directshow_connection_configuration_iterator =
        directshow_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_connection_configuration_iterator != directshow_ui_cb_data_p->configuration->connectionConfigurations.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
      static_cast<struct Test_I_Target_MediaFoundation_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      mediafoundation_connection_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_connection_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->connectionConfigurations.end ());
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
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LISTEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  bool start_listening = gtk_toggle_button_get_active (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LABEL_LISTENING_STRING)
                                         : ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LABEL_LISTEN_STRING)));

  GtkImage* image_p =
    GTK_IMAGE (gtk_builder_get_object ((*iterator).second.second,
                                       (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_IMAGE_CONNECT_NAME)
                                                        : ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_IMAGE_DISCONNECT_NAME))));
  ACE_ASSERT (image_p);
  gtk_button_set_image (GTK_BUTTON (toggle_button_p), GTK_WIDGET (image_p));

  GtkRadioButton* radio_button_p =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME)));
  ACE_ASSERT (radio_button_p);
  enum Net_TransportLayerType protocol = NET_TRANSPORTLAYER_INVALID;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
    protocol = NET_TRANSPORTLAYER_TCP;
  else
    protocol = NET_TRANSPORTLAYER_UDP;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      directshow_ui_cb_data_p->configuration->protocol = protocol;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      mediafoundation_ui_cb_data_p->configuration->protocol = protocol;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  protocol = ui_cb_data_p->configuration->protocol;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Target_DirectShow_InetConnectionManager_t* directshow_connection_manager_p =
    NULL;
  Test_I_Target_MediaFoundation_InetConnectionManager_t* mediafoundation_connection_manager_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_connection_manager_p =
        TEST_I_TARGET_DIRECTSHOW_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (directshow_connection_manager_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_connection_manager_p =
      TEST_I_TARGET_MEDIAFOUNDATION_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (mediafoundation_connection_manager_p);
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
  Test_I_Target_InetConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
#endif
  bool result = false;
  Common_ITask_t* itask_p = NULL;
  if (start_listening)
  {
    switch (protocol)
    {
      case NET_TRANSPORTLAYER_TCP:
      {
        // listening on UDP ? --> stop
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (ui_cb_data_base_p->mediaFramework)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            if (directshow_ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
            {
              Test_I_Target_DirectShow_InetConnectionManager_t::ICONNECTION_T* connection_p =
                directshow_connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (directshow_ui_cb_data_p->configuration->handle));
              if (connection_p)
              {
                connection_p->close ();
                connection_p->decrease ();
              } // end ELSE
              directshow_ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
            } // end IF

            ACE_ASSERT (directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
            itask_p =
              directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
            result =
              directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener->initialize (directshow_ui_cb_data_p->configuration->listenerConfiguration);
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            if (mediafoundation_ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
            {
              Test_I_Target_MediaFoundation_InetConnectionManager_t::ICONNECTION_T* connection_p =
                mediafoundation_connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (mediafoundation_ui_cb_data_p->configuration->handle));
              if (connection_p)
              {
                connection_p->close ();
                connection_p->decrease ();
              } // end ELSE
              mediafoundation_ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
            } // end IF

            ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
            itask_p =
              mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
            result =
              mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener->initialize (mediafoundation_ui_cb_data_p->configuration->listenerConfiguration);
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
        if (ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
            connection_manager_p->get (static_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
          if (connection_p)
          {
            connection_p->close ();
            connection_p->decrease ();
          } // end ELSE
          ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
        } // end IF

        ACE_ASSERT (ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
        itask_p =
          ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
        result =
          ui_cb_data_p->configuration->signalHandlerConfiguration.listener->initialize (ui_cb_data_p->configuration->listenerConfiguration);
#endif
        if (!result)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize listener, returning\n")));
          return;
        } // end IF
        ACE_ASSERT (itask_p);

        try {
          itask_p->start ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_ITask_T::start(): \"%m\", continuing\n")));
        } // end catch

        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      {
        // listening on TCP ? --> stop
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (ui_cb_data_base_p->mediaFramework)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          { ACE_ASSERT (directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
            if (directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener->isRunning ())
              itask_p =
              directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
            if (mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener->isRunning ())
              itask_p =
                mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
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
        ACE_ASSERT (ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
        if (ui_cb_data_p->configuration->signalHandlerConfiguration.listener->isRunning ())
          itask_p =
            ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
#endif
        if (itask_p)
        {
          try {
            itask_p->stop ();
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Common_ITask_T::stop(): \"%m\", continuing\n")));
          } // end catch
        } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (ui_cb_data_base_p->mediaFramework)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            if (directshow_ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
            {
              Test_I_Target_DirectShow_InetConnectionManager_t::ICONNECTION_T* connection_p =
                directshow_connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (directshow_ui_cb_data_p->configuration->handle));
              if (connection_p)
              {
                connection_p->close ();
                connection_p->decrease ();
              } // end ELSE
              directshow_ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
            } // end IF
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            if (mediafoundation_ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
            {
              Test_I_Target_MediaFoundation_InetConnectionManager_t::ICONNECTION_T* connection_p =
                mediafoundation_connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (mediafoundation_ui_cb_data_p->configuration->handle));
              if (connection_p)
              {
                connection_p->close ();
                connection_p->decrease ();
              } // end ELSE
              mediafoundation_ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
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
        if (ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
            connection_manager_p->get (static_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
          if (connection_p)
          {
            connection_p->close ();
            connection_p->decrease ();
          } // end ELSE
          ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
        } // end IF
#endif

        Net_Inet_IConnector_t* iconnector_p = NULL;
        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        ACE_INET_Addr inet_address;
//        int result_2 = -1;
        bool use_reactor =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        Test_I_Target_DirectShow_InetConnectionManager_t::ICONNECTION_T* directshow_connection_p =
          NULL;
        Test_I_Target_MediaFoundation_InetConnectionManager_t::ICONNECTION_T* mediafoundation_connection_p =
          NULL;
        switch (ui_cb_data_base_p->mediaFramework)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            inet_address =
              (*directshow_connection_configuration_iterator).second.socketHandlerConfiguration.socketConfiguration_2.address;
            use_reactor =
              (directshow_ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
            Test_I_Target_DirectShow_InetConnectionManager_t::INTERFACE_T* iconnection_manager_p =
              directshow_connection_manager_p;
            ACE_ASSERT (iconnection_manager_p);
            Test_I_Target_DirectShow_IInetConnector_t* iconnector_2 = NULL;
            if (use_reactor)
              ACE_NEW_NORETURN (iconnector_2,
                                Test_I_Target_DirectShow_UDPConnector_t (iconnection_manager_p,
                                (*mediafoundation_modulehandler_iterator).second.second.statisticReportingInterval));
            else
              ACE_NEW_NORETURN (iconnector_2,
                                Test_I_Target_DirectShow_UDPAsynchConnector_t (iconnection_manager_p,
                                (*directshow_modulehandler_iterator).second.second.statisticReportingInterval));
            if (!iconnector_2)
            {
              ACE_DEBUG ((LM_CRITICAL,
                          ACE_TEXT ("failed to allocate memory, returning\n")));
              return;
            } // end IF
            iconnector_p = iconnector_2;
            result =
              iconnector_2->initialize ((*directshow_connection_configuration_iterator).second);
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            inet_address =
              (*mediafoundation_connection_configuration_iterator).second.socketHandlerConfiguration.socketConfiguration_2.address;
            use_reactor =
              (mediafoundation_ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
            Test_I_Target_MediaFoundation_InetConnectionManager_t::INTERFACE_T* iconnection_manager_p =
              mediafoundation_connection_manager_p;
            ACE_ASSERT (iconnection_manager_p);
            Test_I_Target_MediaFoundation_IInetConnector_t* iconnector_2 = NULL;
            if (use_reactor)
              ACE_NEW_NORETURN (iconnector_2,
                                Test_I_Target_MediaFoundation_UDPConnector_t (iconnection_manager_p,
                                                                              (*mediafoundation_modulehandler_iterator).second.second.statisticReportingInterval));
            else
              ACE_NEW_NORETURN (iconnector_2,
                                Test_I_Target_MediaFoundation_UDPAsynchConnector_t (iconnection_manager_p,
                                                                                    (*directshow_modulehandler_iterator).second.second.statisticReportingInterval));
            if (!iconnector_2)
            {
              ACE_DEBUG ((LM_CRITICAL,
                          ACE_TEXT ("failed to allocate memory, returning\n")));
              return;
            } // end IF
            iconnector_p = iconnector_2;
            result =
              iconnector_2->initialize ((*mediafoundation_connection_configuration_iterator).second);
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
        Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator_2 =
          ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
        ACE_ASSERT (iterator_2 != ui_cb_data_p->configuration->streamConfiguration.end ());

        Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
          NULL;
        Test_I_Target_ConnectionConfigurationIterator_t iterator_3 =
          ui_cb_data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
        ACE_ASSERT (iterator_3 != ui_cb_data_p->configuration->connectionConfigurations.end ());
        inet_address =
          (*iterator_3).second.socketHandlerConfiguration.socketConfiguration_2.address;
        use_reactor =
          (ui_cb_data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0);
        Test_I_Target_InetConnectionManager_t::INTERFACE_T* iconnection_manager_p =
          connection_manager_p;
        ACE_ASSERT (iconnection_manager_p);
        Test_I_Target_IInetConnector_t* iconnector_2 = NULL;
        if (use_reactor)
          ACE_NEW_NORETURN (iconnector_2,
                            Test_I_Target_UDPConnector_t (iconnection_manager_p,
                                                          (*iterator_2).second.second.statisticReportingInterval));
        else
          ACE_NEW_NORETURN (iconnector_2,
                            Test_I_Target_UDPAsynchConnector_t (iconnection_manager_p,
                                                                (*iterator_2).second.second.statisticReportingInterval));
        if (!iconnector_2)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory, returning\n")));
          return;
        } // end IF
        iconnector_p = iconnector_2;
        result = iconnector_2->initialize ((*iterator_3).second);
#endif
        ACE_ASSERT (iconnector_p);
        if (!result)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));

          // clean up
          delete iconnector_p;

          return;
        } // end IF

        // connect
        ACE_HANDLE handle = iconnector_p->connect (inet_address);
        // *TODO*: support one-thread operation by scheduling a signal and manually
        //         running the dispatch loop for a limited time...
        if (!use_reactor)
        {
          // *TODO*: avoid tight loop here
          ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
          //result = ACE_OS::sleep (timeout);
          //if (result == -1)
          //  ACE_DEBUG ((LM_ERROR,
          //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
          //              &timeout));
          ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
          bool done = false;
          do
          {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            switch (ui_cb_data_base_p->mediaFramework)
            {
              case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
                directshow_connection_p =
                  directshow_connection_manager_p->get (inet_address);
                break;
              case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
                mediafoundation_connection_p =
                  mediafoundation_connection_manager_p->get (inet_address);
                break;
              default:
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                            ui_cb_data_base_p->mediaFramework));
                return;
              }
            } // end SWITCH
#else
            connection_p = connection_manager_p->get (inet_address);
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
            switch (ui_cb_data_base_p->mediaFramework)
            {
              case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
              {
                if (mediafoundation_connection_p)
                {
                  handle =
                    reinterpret_cast<ACE_HANDLE> (mediafoundation_connection_p->id ());
                  mediafoundation_connection_p->decrease ();
                  done = true;
                  break;
                } // end IF
                break;
              }
              case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
              {
                if (directshow_connection_p)
                {
                  handle =
                    reinterpret_cast<ACE_HANDLE> (directshow_connection_p->id ());
                  directshow_connection_p->decrease ();
                  done = true;
                  break;
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
#else
            if (connection_p)
            {
              handle = static_cast<ACE_HANDLE> (connection_p->id ());
              connection_p->decrease ();
              done = true;
              break;
            } // end IF
#endif
          } while (!done && (COMMON_TIME_NOW < deadline));
        } // end IF
        if (handle == ACE_INVALID_HANDLE)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to connect to %s, returning\n"),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address).c_str ())));

          // clean up
          delete iconnector_p;

          return;
        } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (ui_cb_data_base_p->mediaFramework)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
            directshow_ui_cb_data_p->configuration->handle = handle;
            break;
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
            mediafoundation_ui_cb_data_p->configuration->handle = handle;
            break;
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                        ui_cb_data_base_p->mediaFramework));
            return;
          }
        } // end SWITCH
#else
        ui_cb_data_p->configuration->handle = handle;
#endif
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%d: started listening (UDP) (%s)...\n"),
                    handle,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address).c_str ())));

        // clean up
        delete iconnector_p;

        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer type (was: %d), returning\n"),
                    protocol));
        return;
      } // end catch
    } // end SWITCH

    GtkFrame* frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

    // start progress reporting
    GtkProgressBar* progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progress_bar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), true);

    ACE_ASSERT (!ui_cb_data_base_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      ui_cb_data_base_p->progressData.eventSourceId =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &ui_cb_data_p->progressData,
        //                 NULL);
        g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                            COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
                            idle_update_progress_target_cb,
                            &ui_cb_data_base_p->progressData,
                            NULL);
      if (ui_cb_data_base_p->progressData.eventSourceId > 0)
        state_r.eventSourceIds.insert (ui_cb_data_base_p->progressData.eventSourceId);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_target_progress_cb): \"%m\", returning\n")));
        return;
      } // end IF
    } // end lock scope
  } // end IF
  else
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      { ACE_ASSERT (directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
        itask_p =
          directshow_ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
        itask_p =
          mediafoundation_ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
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
    ACE_ASSERT (ui_cb_data_p->configuration->signalHandlerConfiguration.listener);
    itask_p =
      ui_cb_data_p->configuration->signalHandlerConfiguration.listener;
#endif
    ACE_ASSERT (itask_p);
    try {
      itask_p->stop ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_ITask_T::stop(): \"%m\", continuing\n")));
    } // end catch

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        if (directshow_ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_DirectShow_InetConnectionManager_t::ICONNECTION_T* connection_p =
            directshow_connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (directshow_ui_cb_data_p->configuration->handle));
          if (connection_p)
          {
            connection_p->close ();
            connection_p->decrease ();
          } // end ELSE
          directshow_ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
        } // end IF
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        if (mediafoundation_ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
        {
          Test_I_Target_MediaFoundation_InetConnectionManager_t::ICONNECTION_T* connection_p =
            mediafoundation_connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (mediafoundation_ui_cb_data_p->configuration->handle));
          if (connection_p)
          {
            connection_p->close ();
            connection_p->decrease ();
          } // end ELSE
          mediafoundation_ui_cb_data_p->configuration->handle =
            ACE_INVALID_HANDLE;
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
    if (ui_cb_data_p->configuration->handle != ACE_INVALID_HANDLE)
    {
      Test_I_Target_InetConnectionManager_t::ICONNECTION_T* connection_p =
        connection_manager_p->get (static_cast<Net_ConnectionId_t> (ui_cb_data_p->configuration->handle));
      if (connection_p)
      {
        connection_p->close ();
        connection_p->decrease ();
      } // end ELSE
      ui_cb_data_p->configuration->handle = ACE_INVALID_HANDLE;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64

    GtkFrame* frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

    // reset info
    idle_reset_target_UI_cb (userData_in);

    // stop progress reporting
    ACE_ASSERT (ui_cb_data_base_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      if (!g_source_remove (ui_cb_data_base_p->progressData.eventSourceId))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    ui_cb_data_base_p->progressData.eventSourceId));
      state_r.eventSourceIds.erase (ui_cb_data_base_p->progressData.eventSourceId);
      ui_cb_data_base_p->progressData.eventSourceId = 0;
    } // end lock scope
    GtkProgressBar* progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progress_bar_p);
    // *NOTE*: this disables "activity mode" (in Gtk2)
    gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
//    gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);
  } // end ELSE
} // action_listen_activate_cb

void
filechooserbutton_target_cb (GtkFileChooserButton* button_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_target_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Target_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Target_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_Target_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Target_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Target_DirectShow_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
      static_cast<struct Test_I_Target_MediaFoundation_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
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
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
#endif

  GFile* file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (button_in));
  ACE_ASSERT (file_p);
  char* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(%@): \"%m\", returning\n"),
                file_p));

    // clean up
    g_object_unref (file_p);

    return;
  } // end IF
  g_object_unref (file_p);

  std::string file_name = Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if (file_name.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));

    // clean up
    g_free (string_p);

    return;
  } // end IF
  g_free (string_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      (*directshow_modulehandler_iterator).second.second.targetFileName =
        file_name;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      (*mediafoundation_modulehandler_iterator).second.second.targetFileName =
        file_name;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  (*iterator).second.second.targetFileName = file_name;
#endif
} // filechooserbutton_target_cb
void
filechooser_target_cb (GtkFileChooser* fileChooser_in,
                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_target_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Target_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Target_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_Target_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Target_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Target_DirectShow_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Target_MediaFoundation_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
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
  struct Test_I_Target_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Target_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
#endif

  GFile* file_p = gtk_file_chooser_get_file (fileChooser_in);
  ACE_ASSERT (file_p);
  char* string_p = g_file_get_path (file_p);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_file_get_path(%@): \"%m\", returning\n"),
                file_p));

    // clean up
    g_object_unref (file_p);

    return;
  } // end IF
  g_object_unref (file_p);

  std::string file_name = Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if (file_name.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));

    // clean up
    g_free (string_p);

    return;
  } // end IF
  g_free (string_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      (*directshow_modulehandler_iterator).second.second.targetFileName =
        file_name;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      (*mediafoundation_modulehandler_iterator).second.second.targetFileName =
        file_name;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  (*iterator).second.second.targetFileName = file_name;
#endif
} // filechooser_target_cb

//////////////////////////////////////////

void
action_report_activate_cb (GtkAction* action_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_report_activate_cb"));

  ACE_UNUSED_ARG (action_in);
  ACE_UNUSED_ARG (userData_in);

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
  int signal = 0;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  signal = SIGUSR1;
#else
  signal = SIGBREAK;
#endif
  if (ACE_OS::raise (signal) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));
} // action_report_activate_cb

//void
//radiobutton_protocol_toggled_cb (GtkToggleButton* toggleButton_in,
//                                 gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::radiobutton_protocol_toggled_cb"));
//
//  // sanity check
//  if (!gtk_toggle_button_get_active (toggleButton_in))
//    return; // nothing to do
//
//  Test_I_GTK_CBData* ui_cb_data_p =
//    static_cast<Test_I_GTK_CBData*> (userData_in);
//
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//    ui_cb_data_p->UIState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//
//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);
//  ACE_ASSERT (ui_cb_data_p->configuration);
//  ACE_ASSERT (iterator != ui_cb_data_p->UIState.builders.end ());
//
//  GtkRadioButton* radio_button_p =
//    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME)));
//  ACE_ASSERT (radio_button_p);
//  const gchar* string_p =
//    gtk_buildable_get_name (GTK_BUILDABLE (radio_button_p));
//  ACE_ASSERT (string_p);
//  if (ACE_OS::strcmp (string_p,
//                      gtk_buildable_get_name (GTK_BUILDABLE (toggleButton_in))) == 0)
//    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_TCP;
//  else
//    ui_cb_data_p->configuration->protocol = NET_TRANSPORTLAYER_UDP;
//}

//void
//spinbutton_port_value_changed_cb (GtkWidget* widget_in,
//                                  gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::spinbutton_port_value_changed_cb"));
//
//  Test_I_GTK_CBData* ui_cb_data_p =
//    static_cast<Test_I_GTK_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);
//  ACE_ASSERT (ui_cb_data_p->configuration);
//
//  unsigned short port_number =
//    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget_in)));
//  ui_cb_data_p->configuration->socketConfiguration.peerAddress.set_port_number (port_number,
//                                                                          1);
//} // spinbutton_port_value_changed_cb

gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (context_in);
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // sanity check(s)
  if (!ui_cb_data_p->pixelBuffer)
    return FALSE; // --> widget has not been realized yet

  //GdkWindow* window_p = gtk_widget_get_window (widget_in);
  //ACE_ASSERT (window_p);
  //GtkAllocation allocation;
  //gtk_widget_get_allocation (widget_in,
  //                           &allocation);
  gdk_cairo_set_source_pixbuf (context_in,
                               ui_cb_data_p->pixelBuffer,
                               0.0, 0.0);

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, FALSE);
    // *IMPORTANT NOTE*: potentially, this involves tranfer of image data to an
    //                   X server running on a different host
    //gdk_draw_pixbuf (GDK_DRAWABLE (window_p), NULL,
    //                 ui_cb_data_p->pixelBuffer,
    //                 0, 0, 0, 0, allocation.width, allocation.height,
    //                 GDK_RGB_DITHER_NONE, 0, 0);
    cairo_paint (context_in);
  } // end lock scope

  return TRUE;
}

// -----------------------------------------------------------------------------

gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  // sanity check(s)
  struct Test_I_GTK_CBData* ui_cb_data_p =
    static_cast<struct Test_I_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME)));
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
  // sanity check(s)
  struct Test_I_GTK_CBData* ui_cb_data_p =
      static_cast<struct Test_I_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
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

  int result = -1;

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);
  //struct Test_I_GTK_CBData* ui_cb_data_p =
  //  static_cast<struct Test_I_GTK_CBData*> (userData_in);
  //// sanity check(s)
  //ACE_ASSERT (ui_cb_data_p);

  //// step1: remove event sources
  //{
  //  ACE_Guard<ACE_Thread_Mutex> aGuard (ui_cb_data_p->UIState.lock);

  //  for (Common_UI_GTKeventSourceIdsIterator_t iterator = ui_cb_data_p->UIState.eventSourceIds.begin ();
  //       iterator != ui_cb_data_p->UIState.eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  ui_cb_data_p->UIState.eventSourceIds.clear ();
  //} // end lock scope

  // step2: initiate shutdown sequence
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int signal = SIGINT;
#else
  int signal = SIGQUIT;
#endif
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, true);

  return FALSE;
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
  struct Test_I_GTK_CBData* ui_cb_data_p =
      static_cast<struct Test_I_GTK_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT(iterator != state_r.builders.end ());

  GtkScrolledWindow* scrolled_window_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SCROLLEDWINDOW_NAME)));
  ACE_ASSERT (scrolled_window_p);
  GtkAdjustment* adjustment_p =
    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p) - gtk_adjustment_get_page_size (adjustment_p));
} // textview_size_allocate_cb

void
combobox_source_changed_cb (GtkComboBox* comboBox_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_source_changed_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

//  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
      NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);

      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (comboBox_in,
                                      &iterator_2))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
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
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string device_identifier_string = g_value_get_string (&value);
  g_value_unset (&value);

  gint n_rows = 0;
  GtkToggleAction* toggle_action_p = NULL;

  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);

  bool result_2 = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::string module_name;
  Stream_Module_t* module_p = NULL;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //module_name =
      //  ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING);
      //module_p =
      //  const_cast<Stream_Module_t*> (directshow_ui_cb_data_p->stream->find (module_name));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //if (mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader)
      //{
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader->Release ();
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader =
      //    NULL;
      //} // end IF
      if ((*mediafoundation_modulehandler_iterator).second.second.mediaSource)
      {
        (*mediafoundation_modulehandler_iterator).second.second.mediaSource->Release (); (*mediafoundation_modulehandler_iterator).second.second.mediaSource = NULL;
      } // end IF
      if ((*mediafoundation_modulehandler_iterator).second.second.session)
      {
        (*mediafoundation_modulehandler_iterator).second.second.session->Release (); (*mediafoundation_modulehandler_iterator).second.second.session = NULL;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Device_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_iterator).second.second.deviceIdentifier,
                                                                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                (*mediafoundation_modulehandler_iterator).second.second.mediaSource))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getMediaSource(\"%s\"), returning\n"),
                    ACE_TEXT ((*mediafoundation_modulehandler_iterator).second.second.deviceIdentifier.c_str ())));
        return;
      } // end IF
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.mediaSource);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      //if (!Stream_Device_Tools::getSourceReader (mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.mediaSource,
      //                                                  symbolic_link_p,
      //                                                  symbolic_link_size,
      //                                                  NULL,
      //                                                  NULL,
      //                                                  false,
      //                                                  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Device_Tools::getSourceReader(\"%s\"), aborting\n"),
      //              ACE_TEXT (mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.device.c_str ())));
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.mediaSource->Release ();
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.mediaSource = NULL;
      //  return;
      //} // end IF
      //ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader);

      module_name =
        ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING);
      module_p =
        const_cast<Stream_Module_t*> (mediafoundation_ui_cb_data_p->stream->find (module_name));
      if (!module_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Base_T::find(\"%s\"), returning\n"),
                    ACE_TEXT (module_name.c_str ())));
        return;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH

  Test_I_Stream_MediaFoundation_CamSource* mediafoundation_source_impl_p = NULL;
  Test_I_Stream_DirectShow_CamSource* directshow_source_impl_p = NULL;
  IMFTopology* topology_p = NULL;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (directshow_ui_cb_data_p->streamConfiguration)
      {
        directshow_ui_cb_data_p->streamConfiguration->Release (); directshow_ui_cb_data_p->streamConfiguration = NULL;
      } // end IF
      if ((*directshow_modulehandler_iterator).second.second.builder)
      {
        (*directshow_modulehandler_iterator).second.second.builder->Release (); (*directshow_modulehandler_iterator).second.second.builder = NULL;
      } // end IF

      IAMBufferNegotiation* buffer_negotiation_p = NULL;
      if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (device_identifier_string,
                                                                   CLSID_VideoInputDeviceCategory,
                                                                   (*directshow_modulehandler_iterator).second.second.builder,
                                                                   buffer_negotiation_p,
                                                                   directshow_ui_cb_data_p->streamConfiguration,
                                                                   (*directshow_stream_iterator).second.configuration_.graphLayout))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                    ACE_TEXT (device_identifier_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.builder);
      ACE_ASSERT (buffer_negotiation_p);
      ACE_ASSERT (directshow_ui_cb_data_p->streamConfiguration);
      buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (module_p);
      mediafoundation_source_impl_p =
        dynamic_cast<Test_I_Stream_MediaFoundation_CamSource*> (module_p->writer ());
      ACE_ASSERT (mediafoundation_source_impl_p);

      struct _MFRatio pixel_aspect_ratio = { 1, 1 };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (device_identifier_string,
                                                                           MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                           (*mediafoundation_modulehandler_iterator).second.second.mediaSource,
                                                                           mediafoundation_source_impl_p,
                                                                           topology_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (topology_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      // sanity check(s)
      ACE_ASSERT (!(*mediafoundation_modulehandler_iterator).second.second.session);

      if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                     (*mediafoundation_modulehandler_iterator).second.second.session,
                                                                     true))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), returning\n")));
        topology_p->Release (); topology_p = NULL;
        return;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      topology_p->Release (); topology_p = NULL;

      if ((*mediafoundation_modulehandler_iterator).second.second.inputFormat)
      {
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->Release (); (*mediafoundation_modulehandler_iterator).second.second.inputFormat = NULL;
      } // end IF
      HRESULT result =
        MFCreateMediaType (&(*mediafoundation_modulehandler_iterator).second.second.inputFormat);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.inputFormat);

      result =
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->SetGUID (MF_MT_MAJOR_TYPE,
                                                                                      MFMediaType_Video);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetGUID(MF_MT_MAJOR_TYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      result =
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->SetUINT32 (MF_MT_INTERLACE_MODE,
                                                                                        MFVideoInterlace_Unknown);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_INTERLACE_MODE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      result =
        MFSetAttributeRatio ((*mediafoundation_modulehandler_iterator).second.second.inputFormat,
                             MF_MT_PIXEL_ASPECT_RATIO,
                             1, 1);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFSetAttributeRatio(MF_MT_PIXEL_ASPECT_RATIO): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  int result_3 = -1;
  if (v4l2_ui_cb_data_p->fileDescriptor != -1)
  {
    result_3 = v4l2_close (v4l2_ui_cb_data_p->fileDescriptor);
    if (result_3 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  v4l2_ui_cb_data_p->fileDescriptor));
    v4l2_ui_cb_data_p->fileDescriptor = -1;
  } // end IFs
  ACE_ASSERT (v4l2_ui_cb_data_p->fileDescriptor == -1);
  Test_I_Source_V4L2_StreamConfigurationsIterator_t iterator_3 =
      v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T iterator_4 =
      (*iterator_3).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_4 != (*iterator_3).second.end ());
  int open_mode =
      (((*iterator_4).second.second.v4l2Method == V4L2_MEMORY_MMAP) ? O_RDWR
                                                                    : O_RDONLY);
  v4l2_ui_cb_data_p->fileDescriptor =
    v4l2_open (device_identifier_string.c_str (),
               open_mode);
  if (v4l2_ui_cb_data_p->fileDescriptor == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", returning\n"),
                ACE_TEXT (device_identifier_string.c_str ()),
                open_mode));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result_2 = load_formats (directshow_ui_cb_data_p->streamConfiguration,
                               list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //if (!load_formats (ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result_2 =
        load_formats ((*mediafoundation_modulehandler_iterator).second.second.mediaSource,
                      list_store_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  result_2 = load_formats (v4l2_ui_cb_data_p->fileDescriptor,
                           list_store_p);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_formats(), returning\n")));
    return;
  } // end IF
  n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                              true);
    gtk_combo_box_set_active (combo_box_p,
                              0);
  } // end IF

  toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TOGGLEACTION_STREAM_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_sensitive (GTK_ACTION (toggle_action_p),
                            true);
} // combobox_source_changed_cb

void
combobox_format_changed_cb (GtkComboBox* comboBox_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator_2 =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator_2 != state_r.builders.end ());

  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (comboBox_in,
                                      &iterator_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator_2).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = Common_Tools::StringToGUID (format_string);
  ACE_ASSERT (!InlineIsEqualGUID (GUID_s, GUID_NULL));
#else
  __u32 format_i = 0;
  std::istringstream converter;
  converter.str (format_string);
  converter >> format_i;
#endif // ACE_WIN32 || ACE_WIN64
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator_2).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);

  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { 
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->streamConfiguration);
      //struct _AMMediaType* media_type_p = NULL;
      //result_2 =
      //  directshow_ui_cb_data_p->streamConfiguration->GetFormat (&media_type_p);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      //  return;
      //} // end IF
      //ACE_ASSERT (media_type_p);
      //media_type_p->subtype = GUID_s;
      //(*directshow_modulehandler_iterator).second.second.inputFormat->subtype =
      //  GUID_s;

      result =
        load_resolutions (directshow_ui_cb_data_p->streamConfiguration,
                          GUID_s,
                          list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.inputFormat);
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.mediaSource);
      //ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.sourceReader);

      result_2 =
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->SetGUID (MF_MT_SUBTYPE,
                                                                                      GUID_s);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF

      //if (!load_resolutions ((*mediafoundation_modulehandler_iterator).second.sourceReader,
      result =
        load_resolutions ((*mediafoundation_modulehandler_iterator).second.second.mediaSource,
                          GUID_s,
                          list_store_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_iterator).second.second.sourceFormat.format.pix.pixelformat =
    format_i;

  result =
    load_resolutions (v4l2_ui_cb_data_p->fileDescriptor,
                      format_i,
                      list_store_p);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_resolutions(), returning\n")));
    return;
  } // end IF
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator_2).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
} // combobox_format_changed_cb

void
combobox_resolution_changed_cb (GtkComboBox* comboBox_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

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
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator_2 =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator_2 != state_r.builders.end ());

  GtkTreeIter iterator_3;
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator_2).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator_2).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s =
    Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (g_value_get_string (&value)));
#else
  __u32 format_i = 0;
  std::istringstream converter;
  converter.str (g_value_get_string (&value));
  converter >> format_i;
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
  if (!gtk_combo_box_get_active_iter (comboBox_in,
                                      &iterator_3))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator_2).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  g_value_init (&value_2, G_TYPE_UINT);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  unsigned int width = g_value_get_uint (&value);
  g_value_unset (&value);
  unsigned int height = g_value_get_uint (&value_2);
  g_value_unset (&value_2);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator_2).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);

  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { 
      // sanity check(s)
      //ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.builder);
      ACE_ASSERT (directshow_ui_cb_data_p->streamConfiguration);

      //struct _AMMediaType* media_type_p = NULL;
      //result_2 = directshow_ui_cb_data_p->streamConfiguration->GetFormat (&media_type_p);
      //if (FAILED (result_2))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      //  return;
      //} // end IF
      //ACE_ASSERT (media_type_p);
      //if (media_type_p->formattype == FORMAT_VideoInfo)
      //{
      if (InlineIsEqualGUID ((*directshow_modulehandler_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo))
      {
        struct tagVIDEOINFOHEADER* video_info_header_p =
          //reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_p->pbFormat);
          reinterpret_cast<struct tagVIDEOINFOHEADER*> ((*directshow_modulehandler_iterator).second.second.inputFormat->pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight = height;
        video_info_header_p->bmiHeader.biSizeImage =
          GetBitmapSize (&video_info_header_p->bmiHeader);
      } // end IF
      else if (InlineIsEqualGUID ((*directshow_modulehandler_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo2))
      {
        // *NOTE*: these media subtypes do not work with the Video Renderer
        //         directly --> insert the Overlay Mixer
        struct tagVIDEOINFOHEADER2* video_info_header2_p =
          //reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_p->pbFormat);
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> ((*directshow_modulehandler_iterator).second.second.inputFormat->pbFormat);
        video_info_header2_p->bmiHeader.biWidth = width;
        video_info_header2_p->bmiHeader.biHeight = height;
        video_info_header2_p->bmiHeader.biSizeImage =
          GetBitmapSize (&video_info_header2_p->bmiHeader);
      } // end ELSE IF
      else
      {


      } // end ELSE

      result = load_rates (directshow_ui_cb_data_p->streamConfiguration,
                           GUID_s,
                           width,
                           list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.inputFormat);
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.mediaSource);
      //ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.sourceReader);

      result_2 =
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->GetGUID (MF_MT_MAJOR_TYPE, //MF_MT_AM_FORMAT_TYPE,
                                                                                      &GUID_s);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_MAJOR_TYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF
      //if ((GUID_s == FORMAT_VideoInfo) ||
      //    (GUID_s == FORMAT_VideoInfo2))
      //{
      ACE_ASSERT (InlineIsEqualGUID (GUID_s, MFMediaType_Video));
      result_2 =
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->GetGUID (MF_MT_SUBTYPE,
                                                                                      &GUID_s);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF

      result_2 =
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->SetUINT32 (MF_MT_SAMPLE_SIZE,
                                                                                        width * height * 4);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_SAMPLE_SIZE,%d): \"%s\", returning\n"),
                    width * height * 4,
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF
      result_2 =
        MFSetAttributeSize ((*mediafoundation_modulehandler_iterator).second.second.inputFormat,
                            MF_MT_FRAME_SIZE,
                            width, height);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFSetAttributeSize(%d,%d): \"%s\", returning\n"),
                    width, height,
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF

      result =
        load_rates (//(*mediafoundation_modulehandler_iterator).second.second.sourceReader,
                    (*mediafoundation_modulehandler_iterator).second.second.mediaSource,
                    GUID_s,
                    width,
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
  (*modulehandler_iterator).second.second.sourceFormat.format.fmt.pix.width =
      width;
  (*modulehandler_iterator).second.second.sourceFormat.format.fmt.pix.height =
      height;

  result = load_rates (v4l2_ui_cb_data_p->fileDescriptor,
                       format_i,
                       width, height,
                       list_store_p);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_rates(), returning\n")));
    return;
  } // end IF

  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator_2).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_COMBOBOX_RATE_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
} // combobox_resolution_changed_cb

void
combobox_rate_changed_cb (GtkComboBox* comboBox_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_rate_changed_cb"));

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator_2 =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator_2 != state_r.builders.end ());

  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (comboBox_in,
                                      &iterator_3))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator_2).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value, value_2;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_UINT);
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  g_value_init (&value_2, G_TYPE_UINT);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            0, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  unsigned int frame_rate = g_value_get_uint (&value);
  g_value_unset (&value);
  unsigned int frame_rate_denominator = g_value_get_uint (&value_2);
  g_value_unset (&value_2);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result = E_FAIL;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { 
      // sanity check(s)
      //ACE_ASSERT ((*directshow_modulehandler_iterator).second.second.builder);
      ACE_ASSERT (directshow_ui_cb_data_p->streamConfiguration);
      //struct _AMMediaType* media_type_p = NULL;
      //HRESULT result = ui_cb_data_p->streamConfiguration->GetFormat (&media_type_p);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      //  return;
      //} // end IF
      //ACE_ASSERT (media_type_p);
      //if (media_type_p->formattype == FORMAT_VideoInfo)
      //if (InlineIsEqualGUID ((*directshow_modulehandler_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo))
      //{
      //  struct tagVIDEOINFOHEADER* video_info_header_p =
      //    //reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_p->pbFormat);
      //    reinterpret_cast<struct tagVIDEOINFOHEADER*> ((*directshow_modulehandler_iterator).second.second.inputFormat->pbFormat);
      //  video_info_header_p->AvgTimePerFrame =
      //    static_cast<REFERENCE_TIME> ((static_cast<double> (frame_rate_denominator) /
      //                                  static_cast<double> (frame_rate)) * 10000000);
      //} // end IF
      //else if (InlineIsEqualGUID ((*directshow_modulehandler_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo2))
      //{
      //  // *NOTE*: these media subtypes do not work with the Video Renderer
      //  //         directly --> insert the Overlay Mixer
      //  struct tagVIDEOINFOHEADER2* video_info_header2_p =
      //    //reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_p->pbFormat);
      //    reinterpret_cast<struct tagVIDEOINFOHEADER2*> ((*directshow_modulehandler_iterator).second.second.inputFormat->pbFormat);
      //  video_info_header2_p->AvgTimePerFrame =
      //    static_cast<REFERENCE_TIME> ((static_cast<double> (frame_rate_denominator) /
      //                                  static_cast<double> (frame_rate)) * 10000000);
      //} // end ELSE IF
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_UNUSED_ARG (frame_rate_denominator);
      // sanity check(s)
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second.inputFormat);

      struct _GUID format_type = GUID_NULL;
      result =
        (*mediafoundation_modulehandler_iterator).second.second.inputFormat->GetGUID (MF_MT_MAJOR_TYPE,
                                                                                      &format_type);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      ACE_ASSERT (InlineIsEqualGUID (format_type, MFMediaType_Video));

      //struct tagPROPVARIANT property_s;
      //PropVariantInit (&property_s);
      //result =
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.format->SetItem (MF_MT_FRAME_RATE,
      //                                                                                     property_s);
      result =
        MFSetAttributeSize ((*mediafoundation_modulehandler_iterator).second.second.inputFormat,
                            MF_MT_FRAME_RATE,
                            frame_rate, 1);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    //ACE_TEXT ("failed to IMFMediaType::SetItem(MF_MT_FRAME_RATE): \"%s\", returning\n"),
                    ACE_TEXT ("failed to MFSetAttributeSize(MF_MT_FRAME_RATE,%u): \"%s\", returning\n"),
                    frame_rate,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

        //// clean up
        //PropVariantClear (&property_s);

        return;
      } // end IF
      //PropVariantClear (&property_s);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_iterator).second.second.sourceFormat.frameRate.numerator =
      frame_rate;
  (*modulehandler_iterator).second.second.sourceFormat.frameRate.denominator =
      frame_rate_denominator;
#endif // ACE_WIN32 || ACE_WIN64
  set_capture_format (ui_cb_data_p);
  update_buffer_size (ui_cb_data_p);
} // combobox_rate_changed_cb

#if GTK_CHECK_VERSION(3,0,0)
void
drawingarea_size_allocate_source_cb (GtkWidget* widget_in,
                                     GdkRectangle* allocation_in,
                                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_size_allocate_source_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (allocation_in);
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (v4l2_ui_cb_data_p->configuration);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

  //if (!ui_cb_data_p->configuration->moduleHandlerConfiguration.window) // <-- window not realized yet ?
  //  return;

  //Common_UI_GTK_BuildersConstIterator_t iterator =
  //  ui_cb_data_p->UIState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != ui_cb_data_p->UIState.builders.end ());


  //GtkDrawingArea* drawing_area_p =
  //  GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  //ACE_ASSERT (drawing_area_p);
  //GtkAllocation allocation;
  //ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
  //gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
  //                           &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {//ACE_ASSERT (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration->windowController);
      (*directshow_modulehandler_iterator).second.second.area.bottom =
        allocation_in->height;
      (*directshow_modulehandler_iterator).second.second.area.left =
        allocation_in->x;
      (*directshow_modulehandler_iterator).second.second.area.right =
        allocation_in->width;
      (*directshow_modulehandler_iterator).second.second.area.top =
        allocation_in->y;

      //HRESULT result =
      //  ui_cb_data_p->configuration.moduleHandlerConfiguration->windowController->SetWindowPosition (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom);
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom,
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_iterator).second.second.area.bottom =
        allocation_in->height;
      (*mediafoundation_modulehandler_iterator).second.second.area.left =
        allocation_in->x;
      (*mediafoundation_modulehandler_iterator).second.second.area.right =
        allocation_in->width;
      (*mediafoundation_modulehandler_iterator).second.second.area.top =
        allocation_in->y;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_iterator).second.second.area.width = allocation_in->width;
  (*modulehandler_iterator).second.second.area.height = allocation_in->height;
#endif
} // drawingarea_size_allocate_source_cb
void
drawingarea_size_allocate_target_cb (GtkWidget* widget_in,
                                     GdkRectangle* allocation_in,
                                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_size_allocate_target_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (allocation_in);

  // sanity check(s)
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

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
  struct Test_I_Source_V4L2_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { //ACE_ASSERT (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration->windowController);
      //gint x, y;
      //gtk_widget_translate_coordinates (widget_in,
      //                                  gtk_widget_get_toplevel (widget_in),
      //                                  0, 0, &x, &y);
      (*directshow_modulehandler_iterator).second.second.area.left =
        allocation_in->x;
      (*directshow_modulehandler_iterator).second.second.area.top =
        allocation_in->y;
      (*directshow_modulehandler_iterator).second.second.area.right =
        allocation_in->x + allocation_in->width;
      (*directshow_modulehandler_iterator).second.second.area.bottom =
        allocation_in->y + allocation_in->height;

      //HRESULT result =
      //  ui_cb_data_p->configuration.moduleHandlerConfiguration->windowController->SetWindowPosition (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom);
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom,
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_iterator).second.second.area.bottom =
        allocation_in->height;
      (*mediafoundation_modulehandler_iterator).second.second.area.left =
        allocation_in->x;
      (*mediafoundation_modulehandler_iterator).second.second.area.right =
        allocation_in->width;
      (*mediafoundation_modulehandler_iterator).second.second.area.top =
        allocation_in->y;
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
  Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  (*iterator).second.second.area = *allocation_in;
#endif
} // drawingarea_size_allocate_target_cb
#else
gboolean
drawingarea_configure_event_source_cb (GtkWidget* widget_in,
                                       GdkEvent* event_in,
                                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_source_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (event_in);
  struct Test_I_CamStream_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Source_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;

  Test_I_Source_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Source_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_stream_iterator =
        directshow_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_ui_cb_data_p->configuration->streamConfigurations.end ());
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Source_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_stream_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfigurations.end ());
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_I_Source_V4L2_UI_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Source_V4L2_UI_CBData*> (userData_in);
  ACE_ASSERT (v4l2_ui_cb_data_p);
  ACE_ASSERT (v4l2_ui_cb_data_p->configuration);

  Test_I_Source_V4L2_StreamConfigurationsIterator_t stream_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != v4l2_ui_cb_data_p->configuration->streamConfigurations.end ());
  Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { //ACE_ASSERT (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration->windowController);
      (*directshow_modulehandler_iterator).second.second.area.bottom =
        event_in->configure.height;
      (*directshow_modulehandler_iterator).second.second.area.left =
        event_in->configure.x;
      (*directshow_modulehandler_iterator).second.second.area.right =
        event_in->configure.width;
      (*directshow_modulehandler_iterator).second.second.area.top =
        event_in->configure.y;

      //HRESULT result =
      //  ui_cb_data_p->configuration.moduleHandlerConfiguration->windowController->SetWindowPosition (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom);
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom,
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_iterator).second.second.area.bottom =
        event_in->configure.height;
      (*mediafoundation_modulehandler_iterator).second.second.area.left =
        event_in->configure.x;
      (*mediafoundation_modulehandler_iterator).second.second.area.right =
        event_in->configure.width;
      (*mediafoundation_modulehandler_iterator).second.second.area.top =
        event_in->configure.y;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  (*modulehandler_iterator).second.second.area.height =
      event_in->configure.height;
  (*modulehandler_iterator).second.second.area.width =
      event_in->configure.width;
#endif

  return FALSE;
} // drawingarea_configure_event_source_cb
gboolean
drawingarea_configure_event_target_cb (GtkWidget* widget_in,
                                       GdkEvent* event_in,
                                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_target_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (event_in);
  struct Test_I_CamStream_UI_CBData* ui_cb_data_base_p =
    static_cast<struct Test_I_CamStream_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Target_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_I_Target_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_I_Target_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  Test_I_Target_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  switch (cb_ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_I_Target_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_I_Target_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  cb_ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  struct Test_I_Target_V4L2_GTK_CBData* v4l2_ui_cb_data_p =
    static_cast<struct Test_I_Target_V4L2_GTK_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (v4l2_ui_cb_data_p);
  ACE_ASSERT (v4l2_ui_cb_data_p->configuration);
  Test_I_Target_V4L2_StreamConfiguration_t::ITERATOR_T modulehandler_iterator =
    v4l2_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != v4l2_ui_cb_data_p->configuration->streamConfiguration.end ());
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { //ACE_ASSERT (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration->windowController);
      (*directshow_modulehandler_iterator).second.second.area.bottom =
        event_in->configure.height;
      (*directshow_modulehandler_iterator).second.second.area.left =
        event_in->configure.x;
      (*directshow_modulehandler_iterator).second.second.area.right =
        event_in->configure.width;
      (*directshow_modulehandler_iterator).second.second.area.top =
        event_in->configure.y;

      //HRESULT result =
      //  ui_cb_data_p->configuration.moduleHandlerConfiguration->windowController->SetWindowPosition (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right,
      //                                                                                         directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom);
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.left, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.top,
      //              directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.area.right, ui_cb_data_p->configuration->moduleHandlerConfiguration.area.bottom,
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_iterator).second.second.area.bottom =
        event_in->configure.height;
      (*mediafoundation_modulehandler_iterator).second.second.area.left =
        event_in->configure.x;
      (*mediafoundation_modulehandler_iterator).second.second.area.right =
        event_in->configure.width;
      (*mediafoundation_modulehandler_iterator).second.second.area.top =
        event_in->configure.y;
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
  (*modulehandler_iterator).second.second.area.height =
    event_in->configure.height;
  (*modulehandler_iterator).second.second.area.width =
    event_in->configure.width;
#endif

  return FALSE;
} // drawingarea_configure_event_target_cb
#endif

/////////////////////////////////////////

void
filechooserdialog_target_cb (GtkFileChooser* chooser_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserdialog_target_cb"));

  ACE_UNUSED_ARG (userData_in);

//  Test_I_GTK_CBData* ui_cb_data_p =
//    static_cast<Test_I_GTK_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (ui_cb_data_p);

  gtk_dialog_response (GTK_DIALOG (GTK_FILE_CHOOSER_DIALOG (chooser_in)),
                       GTK_RESPONSE_ACCEPT);
} // filechooserdialog_cb

#ifdef __cplusplus
}
#endif /* __cplusplus */
