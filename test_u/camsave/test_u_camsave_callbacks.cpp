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
#include "test_u_camsave_callbacks.h"

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
#include <dshow.h>
#include <dvdmedia.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
//#include <streams.h>

#include "gdk/gdkkeysyms.h"
#include "gdk/gdkwin32.h"
#else
#include "linux/videodev2.h"
#include "libv4l2.h"

#include "ace/Dirent_Selector.h"
#endif

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_mediafoundation_tools.h"
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_mediafoundation_tools.h"
#include "stream_lib_tools.h"
#endif

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_defines.h"
#include "test_u_camsave_stream.h"

// global variables
bool un_toggling_stream = false;

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
dirent_comparator(const dirent** d1,
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
                    ACE_TEXT (Common_Tools::errorToString (result_2, false).c_str ())));
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
                    ACE_TEXT (Common_Tools::errorToString (result_2, false).c_str ())));
        //result_2 = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        goto error;
      } // end IF
      ACE_ASSERT (enum_moniker_p);
      enumerator_p->Release ();
      enumerator_p = NULL;

      VariantInit (&variant_s);
      while (enum_moniker_p->Next (1, &moniker_p, NULL) == S_OK)
      { ACE_ASSERT (moniker_p);
        properties_p = NULL;
        result = moniker_p->BindToStorage (0, 0, IID_PPV_ARGS (&properties_p));
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
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
                      ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
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
                      ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
          goto error;
        } // end IF
        properties_p->Release ();
        properties_p = NULL;
        ACE_Wide_To_Ascii converter_2 (variant_s.bstrVal);
        result_2 = VariantClear (&variant_s);
        ACE_ASSERT (SUCCEEDED (result_2));

        listbox_entries_a.push_back (std::make_pair (friendly_name_string,
                                                     converter_2.char_rep ()));

        moniker_p->Release ();
        moniker_p = NULL;
      } // end WHILE
      enum_moniker_p->Release ();
      enum_moniker_p = NULL;

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
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error_2;
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
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error_2;
      } // end IF
#else
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
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
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
          goto error_2;
        } // end IF
        listbox_entries_a.push_back (std::make_pair (friendly_name_string,
                                                     ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a))));
#else
        ACE_ASSERT (false);
        ACE_NOTSUP_RETURN (false);
        ACE_NOTREACHED (return false;)
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
  std::string directory (ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY));
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
#endif

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
  std::string media_subtype_string;
  std::string GUID_stdstring;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities_s;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) &&
        !InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    {
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF

    // *NOTE*: FORMAT_VideoInfo2 types do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    GUIDs.insert (media_type_p->subtype);

    Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  for (std::set<struct _GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    media_subtype_string =
      Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2,
                                                         STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
    gtk_list_store_set (listStore_in, &iterator,
                        0, media_subtype_string.c_str (),
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

  HRESULT result = S_OK;
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    // clean up
    stream_descriptor_p->Release ();

    return false;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

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
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  GtkTreeIter iterator;
  for (std::set<struct _GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    media_subtype_string =
      Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2,
                                                         STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION);
    gtk_list_store_set (listStore_in, &iterator,
                        0, media_subtype_string.c_str (),
                        1, Common_Tools::GUIDToString (*iterator_2).c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
load_resolutions (IAMStreamConfig* IAMStreamConfig_in,
                  REFGUID mediaSubType_in,
                  GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_resolutions"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (!InlineIsEqualGUID (mediaSubType_in, GUID_NULL));
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities_s;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  std::set<std::pair<unsigned int, unsigned int> > resolutions;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in))
    {
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
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
      video_info_header2_p =
        (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      resolutions.insert (std::make_pair (video_info_header_p->bmiHeader.biWidth,
                                          video_info_header_p->bmiHeader.biHeight));
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (media_type_p->formattype).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      continue;
    } // end ELSE
    Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
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
                  REFGUID mediaSubType_in,
                  GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_resolutions"));

  // sanity check(s)
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (IMFMediaSource_in);
  ACE_ASSERT (!InlineIsEqualGUID (mediaSubType_in, GUID_NULL));
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    // clean up
    stream_descriptor_p->Release ();

    return false;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
  UINT32 width, height;
  do
  {
    media_type_p = NULL;
    result =
      //IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      //                                        count,
      //                                        &media_type_p);
      media_type_handler_p->GetMediaTypeByIndex (count,
                                                 &media_type_p);
    if (!SUCCEEDED (result))
      break;
    ACE_ASSERT (media_type_p);

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF
    if (!InlineIsEqualGUID (GUID_s, mediaSubType_in))
    {
      media_type_p->Release ();
      ++count;
      continue;
    } // end IF

    result = MFGetAttributeSize (media_type_p,
                                 MF_MT_FRAME_SIZE,
                                 &width, &height);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF
    resolutions.insert (std::make_pair (width, height));
    media_type_p->Release ();
    ++count;
  } while (true);
  media_type_handler_p->Release ();
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
            REFGUID mediaSubType_in,
            unsigned int width_in,
            GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_rates"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (!InlineIsEqualGUID (mediaSubType_in, GUID_NULL));
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities_s;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  unsigned int frame_duration;
  std::set<std::pair<unsigned int, unsigned int> > frame_rates;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in))
    {
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      if (video_info_header_p->bmiHeader.biWidth != width_in)
      {
        Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
        continue;
      } // end IF
      frame_duration =
        static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end IF
    else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    {
      video_info_header2_p = (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      if (video_info_header2_p->bmiHeader.biWidth != width_in)
      {
        Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
        continue;
      } // end IF
      frame_duration =
        static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end ELSEIF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (media_type_p->formattype).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    frame_rates.insert (std::make_pair ((10000000 / frame_duration), 1));
    Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
  } // end WHILE

  std::ostringstream converter;
  std::string frame_rate_string;
  GtkTreeIter iterator;
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = frame_rates.begin ();
       iterator_2 != frame_rates.end ();
       ++iterator_2)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << (double)(*iterator_2).first / (double)(*iterator_2).second;
    frame_rate_string = converter.str ();

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, frame_rate_string.c_str (),
                        1, (*iterator_2).first,
                        2, (*iterator_2).second,
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
  ACE_ASSERT (!InlineIsEqualGUID (mediaSubType_in, GUID_NULL));
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  std::set<std::pair<unsigned int, unsigned int> > frame_rates;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    // clean up
    stream_descriptor_p->Release ();

    return false;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = { 0 };
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
    if (result != S_OK)
      break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF
    result = MFGetAttributeSize (media_type_p,
                                 MF_MT_FRAME_SIZE,
                                 &width, &height);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

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
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

        // clean up
        media_type_handler_p->Release ();
        media_type_p->Release ();

        return false;
      } // end IF
      frame_rates.insert (std::make_pair (numerator, denominator));
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  std::ostringstream converter;
  std::string frame_rate_string;
  GtkTreeIter iterator;
  for (std::set<std::pair<unsigned int, unsigned int> >::const_iterator iterator_2 = frame_rates.begin ();
       iterator_2 != frame_rates.end ();
       ++iterator_2)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << (double)(*iterator_2).first / (double)(*iterator_2).second;
    frame_rate_string = converter.str ();

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, frame_rate_string.c_str (),
                        1, (*iterator_2).first,
                        2, (*iterator_2).second,
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
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                    fd_in, ACE_TEXT ("VIDIOC_ENUM_FRAMEINTERVALS")));
      break;
    } // end IF
    ++frame_interval_description.index;

    if (frame_interval_description.type != V4L2_FRMIVAL_TYPE_DISCRETE)
      continue;

    frame_intervals.insert (frame_interval_description.discrete);
  } while (true);

  std::ostringstream converter;
  std::string frame_rate_string;
  GtkTreeIter iterator;
  for (std::set<struct v4l2_fract, less_fract>::const_iterator iterator_2 = frame_intervals.begin ();
       iterator_2 != frame_intervals.end ();
       ++iterator_2)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << ((double)(*iterator_2).denominator /
                  (double)(*iterator_2).numerator);
    frame_rate_string = converter.str ();

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, frame_rate_string.c_str (),
                        1, (*iterator_2).numerator,
                        2, (*iterator_2).denominator,
                        -1);
  } // end FOR

  return true;
}
#endif

void
set_capture_format (struct Stream_CamSave_GTK_CBData* CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::set_capture_format"));

  // sanity check(s)
  ACE_ASSERT (CBData_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    CBData_in->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != CBData_in->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (CBData_in);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (CBData_in);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
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
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (CBData_in);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);

#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
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
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
    return; // <-- nothing selected
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      ACE_ASSERT ((*directshow_stream_iterator).second.second.builder);

      // step1: set capture format
      if ((*directshow_stream_iterator).second.second.sourceFormat)
        Stream_MediaFramework_DirectShow_Tools::deleteMediaType ((*directshow_stream_iterator).second.second.sourceFormat);
      if (!Stream_Module_Device_DirectShow_Tools::getVideoCaptureFormat ((*directshow_stream_iterator).second.second.builder,
                                                                         media_subtype,
                                                                         width, height,
                                                                         (*directshow_stream_iterator).second.second.sourceFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::getVideoCaptureFormat(%s,%u,%u), returning\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                    width, height));
        return;
      } // end IF
      ACE_ASSERT ((*directshow_stream_iterator).second.second.sourceFormat);
      if (!Stream_Module_Device_DirectShow_Tools::setCaptureFormat ((*directshow_stream_iterator).second.second.builder,
                                                                    CLSID_VideoInputDeviceCategory,
                                                                    *(*directshow_stream_iterator).second.second.sourceFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::setCaptureFormat(), returning\n")));
        return;
      } // end IF

      // step2: adjust output format
      // sanity check(s)
      ACE_ASSERT ((*directshow_stream_iterator).second.second.inputFormat);
      if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo))
      { ACE_ASSERT ((*directshow_stream_iterator).second.second.inputFormat->cbFormat == sizeof (struct tagVIDEOINFOHEADER));
        struct tagVIDEOINFOHEADER* video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> ((*directshow_stream_iterator).second.second.inputFormat->pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight = height;
        video_info_header_p->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_p->bmiHeader);
        ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
        video_info_header_p->dwBitRate =
          (video_info_header_p->bmiHeader.biSizeImage * 8) *                      // bits / frame
          (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

        (*directshow_stream_iterator).second.second.inputFormat->lSampleSize =
          video_info_header_p->bmiHeader.biSizeImage;
      } // end IF
      else if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo2))
      { ACE_ASSERT ((*directshow_stream_iterator).second.second.inputFormat->cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
        struct tagVIDEOINFOHEADER2* video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> ((*directshow_stream_iterator).second.second.inputFormat->pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight = height;
        video_info_header_p->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_p->bmiHeader);
        ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
        video_info_header_p->dwBitRate =
          (video_info_header_p->bmiHeader.biSizeImage * 8) *                      // bits / frame
          (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

        (*directshow_stream_iterator).second.second.inputFormat->lSampleSize =
          video_info_header_p->bmiHeader.biSizeImage;
      } // end IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString ((*directshow_stream_iterator).second.second.inputFormat->formattype).c_str ())));
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
#endif
}

void
update_buffer_size (struct Stream_CamSave_GTK_CBData* CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::update_buffer_size"));

  // sanity check(s)
  ACE_ASSERT (CBData_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    CBData_in->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != CBData_in->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (CBData_in);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (CBData_in);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
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
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (CBData_in);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  unsigned int frame_size_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (CBData_in->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT ((*directshow_stream_iterator).second.second.sourceFormat);
      frame_size_i = 
        Stream_MediaFramework_Tools::frameSize (*(*directshow_stream_iterator).second.second.sourceFormat);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.sourceFormat);
      struct _GUID media_subtype = GUID_NULL;
      HRESULT result =
        (*mediafoundation_stream_iterator).second.second.sourceFormat->GetGUID (MF_MT_SUBTYPE,
                                                                                &media_subtype);
      ACE_ASSERT (SUCCEEDED (result));
      UINT32 width, height;
      result =
        MFGetAttributeSize ((*mediafoundation_stream_iterator).second.second.sourceFormat,
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
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
  frame_size_i = (*iterator_2).second.second.inputFormat.fmt.pix.sizeimage;
#endif
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<gdouble> (frame_size_i));
}

//////////////////////////////////////////

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("processing thread (id: %t) starting...\n")));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif

  struct Test_U_GTK_ThreadData* data_base_p =
      static_cast<struct Test_U_GTK_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);
  ACE_ASSERT (data_base_p->CBData);

  Common_UI_GTK_BuildersIterator_t iterator;
    //  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  std::ostringstream converter;
  //ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->CBData->lock);
  const Stream_CamSave_SessionData_t* session_data_container_p = NULL;
  const Stream_CamSave_SessionData* session_data_p = NULL;
  Stream_IStreamControlBase* stream_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  switch (data_base_p->CBData->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p->CBData);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      ACE_ASSERT (directshow_cb_data_p->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p->CBData);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_cb_data_p->stream);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->CBData->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p->CBData);
  ACE_ASSERT (cb_data_p->configuration);
  ACE_ASSERT (cb_data_p->stream);
#endif

  iterator =
    data_base_p->CBData->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_base_p->CBData->builders.end ());

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->CBData->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T iterator =
        const_cast<Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T&> (directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR ("")));
      ACE_ASSERT (iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      if ((*iterator).second.second.direct3DDevice)
      {
        (*iterator).second.second.direct3DDevice->Release ();
        (*iterator).second.second.direct3DDevice = NULL;
      } // end IF

      if (!directshow_cb_data_p->stream->initialize (directshow_cb_data_p->configuration->streamConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, aborting\n")));
        goto error;
      } // end IF
      stream_p = directshow_cb_data_p->stream;
      session_data_container_p = &directshow_cb_data_p->stream->getR ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (!mediafoundation_cb_data_p->stream->initialize (mediafoundation_cb_data_p->configuration->streamConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, aborting\n")));
        goto error;
      } // end IF
      stream_p = mediafoundation_cb_data_p->stream;
      session_data_container_p = &mediafoundation_cb_data_p->stream->getR ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->CBData->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  if (!cb_data_p->stream->initialize (cb_data_p->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_CamSave_Stream::initialize(), aborting\n")));
    goto error;
  } // end IF
  stream_p = cb_data_p->stream;
  session_data_container_p = &cb_data_p->stream->getR ();
#endif
  ACE_ASSERT (session_data_container_p);
  session_data_p = &session_data_container_p->getR ();
  data_base_p->sessionId = session_data_p->sessionId;
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << session_data_p->sessionId;

  // generate context id
  gdk_threads_enter ();
  data_base_p->CBData->contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                                          gtk_statusbar_get_context_id (statusbar_p,
                                                                                        converter.str ().c_str ())));
  gdk_threads_leave ();

  // *NOTE*: blocks until 'finished'
  ACE_ASSERT (stream_p);
  stream_p->start ();
  ACE_ASSERT (!stream_p->isRunning ());
  //stream_p->wait (true, false, false);

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
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_base_p->CBData->lock, -1);
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_base_p->CBData->lock, std::numeric_limits<void*>::max ());
#endif
    data_base_p->CBData->progressData.completedActions.insert (data_base_p->eventSourceId);
  } // end lock scope

  // clean up
  delete data_base_p;

  return result;
}

/////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_base_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT hresult = CoInitializeEx (NULL, COINIT_MULTITHREADED);
  if (FAILED (hresult))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (hresult).c_str ())));
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

  GtkWidget* about_dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

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
  if (!load_capture_devices (data_base_p->mediaFramework,
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
                                        1, GTK_SORT_DESCENDING);
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
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
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RATE_NAME)));
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
  std::string filename_string;
  bool is_fullscreen_b = false;
  unsigned int buffer_size_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      filename_string =
        (*directshow_stream_iterator).second.second.targetFileName;
      is_fullscreen_b = (*directshow_stream_iterator).second.second.fullScreen;
      buffer_size_i =
        directshow_cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      filename_string =
        (*mediafoundation_stream_iterator).second.second.targetFileName;
      is_fullscreen_b =
        (*mediafoundation_stream_iterator).second.second.fullScreen;
      buffer_size_i =
        mediafoundation_cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
  filename_string = (*iterator_2).second.second.targetFileName;
  is_fullscreen_b = (*iterator_2).second.second.fullScreen;
  buffer_size_i =
    cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize;
#endif
  if (!filename_string.empty ())
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
    //file_p = g_file_new_for_path (filename_string.c_str ());
    //ACE_ASSERT (file_p);
    //ACE_ASSERT (g_file_query_exists (file_p, NULL));

    //std::string file_uri =
    //  ACE_TEXT_ALWAYS_CHAR ("file://") +
    //  filename_string;
    //if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
    //                                              file_uri.c_str ()))
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename_string.c_str ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (filename_string.c_str ())));
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

  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_SAVE_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_toggle_action_set_active (toggle_action_p,
                                !filename_string.empty ());

  toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_FULLSCREEN_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_toggle_action_set_active (toggle_action_p,
                                is_fullscreen_b);

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<gdouble> (buffer_size_i));

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
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_base_p->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log views
    guint event_source_id = g_timeout_add_seconds (1,
                                                   idle_update_log_display_cb,
                                                   userData_in);
    if (event_source_id > 0)
      data_base_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
    // schedule asynchronous updates of the info view
    event_source_id = g_timeout_add (COMMON_UI_GTK_INTERVAL_DEFAULT_WIDGET_REFRESH,
                                     idle_update_info_display_cb,
                                     userData_in);
    if (event_source_id > 0)
      data_base_p->eventSourceIds.insert (event_source_id);
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

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               data_base_p);

  // step6a: connect default signals
  gulong result_2 =
      g_signal_connect (dialog_p,
                        ACE_TEXT_ALWAYS_CHAR ("destroy"),
                        G_CALLBACK (gtk_widget_destroyed),
                        NULL);
  ACE_ASSERT (result_2);

  result_2 = g_signal_connect_swapped (G_OBJECT (about_dialog_p),
                                       ACE_TEXT_ALWAYS_CHAR ("response"),
                                       G_CALLBACK (gtk_widget_hide),
                                       about_dialog_p);
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

  // set defaults
  //file_chooser_button_p =
  //  GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri += filename_string;
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
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (allocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT (window_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (!(*directshow_stream_iterator).second.second.window);
      ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
      (*directshow_stream_iterator).second.second.window =
        gdk_win32_window_get_impl_hwnd (window_p);
        //static_cast<HWND> (GDK_WINDOW_HWND (window_p));
      ACE_ASSERT (IsWindow ((*directshow_stream_iterator).second.second.window));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("drawing area window handle: %@\n"),
                  (*directshow_stream_iterator).second.second.window));
        
      (*directshow_stream_iterator).second.second.area.bottom =
        allocation.y + allocation.height;
      (*directshow_stream_iterator).second.second.area.left = allocation.x;
      (*directshow_stream_iterator).second.second.area.right =
        allocation.x + allocation.width;
      (*directshow_stream_iterator).second.second.area.top = allocation.y;

      //(*directshow_stream_iterator).second.second.pixelBuffer =
      //  data_base_p->pixelBuffer;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (!(*mediafoundation_stream_iterator).second.second.window);
      ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
      (*mediafoundation_stream_iterator).second.second.window =
        gdk_win32_window_get_impl_hwnd (window_p);
      //  static_cast<HWND> (GDK_WINDOW_HWND (GDK_DRAWABLE (window_p)));

      (*mediafoundation_stream_iterator).second.second.area.bottom =
        allocation.y + allocation.height;
      (*mediafoundation_stream_iterator).second.second.area.left = allocation.x;
      (*mediafoundation_stream_iterator).second.second.area.right =
        allocation.x + allocation.width;
      (*mediafoundation_stream_iterator).second.second.area.top = allocation.y;

      //(*mediafoundation_stream_iterator).second.second.pixelBuffer =
      //  data_base_p->pixelBuffer;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  ACE_ASSERT (!cb_data_p->pixelBuffer);
  cb_data_p->pixelBuffer =
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
      gdk_pixbuf_get_from_window (window_p,
                                  0, 0,
                                  allocation.width, allocation.height);
#else
    gdk_pixbuf_get_from_drawable (NULL,
                                  GDK_DRAWABLE (window_p),
                                  NULL,
                                  0, 0,
                                  0, 0, allocation.width, allocation.height);
#endif
  if (!cb_data_p->pixelBuffer)
  { // *NOTE*: most probable reason: window is not mapped
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_drawable(), aborting\n")));
#endif
    return G_SOURCE_REMOVE;
  } // end IF

  ACE_ASSERT (!(*iterator_2).second.second.window);
  (*iterator_2).second.second.window =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT ((*iterator_2).second.second.window);
  (*iterator_2).second.second.area = allocation;
  (*iterator_2).second.second.pixelBuffer = cb_data_p->pixelBuffer;
#endif

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

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

//  struct Stream_CamSave_GTK_CBData* cb_data_base_p =
//    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (userData_in);

  // clean up
  int result = -1;
  if (cb_data_p->fileDescriptor != -1)
  {
    result = v4l2_close (cb_data_p->fileDescriptor);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  cb_data_p->fileDescriptor));
    cb_data_p->fileDescriptor = -1;
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

  struct Stream_CamSave_GTK_CBData* data_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
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
  gtk_action_set_stock_id (GTK_ACTION (toggle_action_p),
                           GTK_STOCK_MEDIA_RECORD);
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
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_SNAPSHOT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, false);

  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  //// stop progress reporting
  //ACE_ASSERT (data_p->progressEventSourceId);
  //{
  //  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard_2, data_p->lock, G_SOURCE_REMOVE);

  //  if (!g_source_remove (data_p->progressEventSourceId))
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                data_p->progressEventSourceId));
  //  data_p->eventSourceIds.erase (data_p->progressEventSourceId);
  //  data_p->progressEventSourceId = 0;

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

  struct Stream_CamSave_GTK_CBData* data_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
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
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);

    // sanity check
    if (data_p->logStack.empty ()) return G_SOURCE_CONTINUE;

    // step1: convert text
    for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
         iterator_2 != data_p->logStack.end ();
         iterator_2++)
    {
      converted_text = Common_UI_GTK_Tools::localeToUTF8 (*iterator_2);
      if (!converted_text)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"), aborting\n"),
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

  struct Stream_CamSave_GTK_CBData* data_p =
      static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (data_p->eventStack);
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
                                     static_cast<gdouble> (data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          break;
        }
        case COMMON_UI_EVENT_FINISHED:
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.capturedFrames));
#endif

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.droppedFrames));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
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
    while (!data_p->eventStack.is_empty ())
    {
      result = data_p->eventStack.pop (event_e);
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

  struct Stream_CamSave_GTK_ProgressData* data_p =
      static_cast<struct Stream_CamSave_GTK_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->state);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);

  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
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
    ACE_thread_t thread_id = (*iterator_2).second.id ();
    result = thread_manager_p->join (thread_id, &exit_status);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                  thread_id));
    else
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: %u)\n"),
                  thread_id,
                  exit_status));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: 0x%@)\n"),
                  thread_id,
                  exit_status));
#endif
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
idle_update_video_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_video_display_cb"));

  struct Stream_CamSave_GTK_CBData* data_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
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
  struct Stream_CamSave_GTK_CBData* data_p =
      static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
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
                            gtk_adjustment_get_upper (adjustment_p) - gtk_adjustment_get_page_size (adjustment_p));
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

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      stream_p = directshow_cb_data_p->stream;
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      stream_p = mediafoundation_cb_data_p->stream;
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  stream_p = cb_data_p->stream;
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (stream_p);
  ACE_ASSERT (iterator != data_base_p->builders.end ());

  GtkAction* action_p = NULL;
  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);

  // toggle ?
  if (!gtk_toggle_action_get_active (toggleAction_in))
  {
    // --> user pressed pause/stop

    // stop stream
    stream_p->stop (false, // wait ?
                    true); // locked access ?

    return;
  } // end IF

  // --> user pressed record

  struct Test_U_GTK_ThreadData* thread_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;

  GtkSpinButton* spin_button_p = NULL;
  unsigned int buffer_size_i = 0;
  gdouble value_d = 0.0;

  if (data_base_p->isFirst)
    data_base_p->isFirst = false;

  // step0: modify widgets
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in),
                           GTK_STOCK_MEDIA_STOP);

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
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_SNAPSHOT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);

  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
  //                          false);

  // step1: set up progress reporting
  ACE_OS::memset (&data_base_p->progressData.statistic,
                  0,
                  sizeof (struct Stream_CamSave_StatisticData));
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  //gint width, height;
  //gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  //gtk_progress_bar_set_pulse_step (progress_bar_p,
  //                                 1.0 / static_cast<double> (width));
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), TRUE);

  // step2: update configuration
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_3;
  if (gtk_combo_box_get_active_iter (combo_box_p,
                                     &iterator_3))
  {
    GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
    ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_3,
                              1, &value);
    ACE_ASSERT (G_VALUE_HOLDS (&value, G_TYPE_STRING));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      (*directshow_stream_iterator).second.second.deviceIdentifier =
        g_value_get_string (&value);
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      (*mediafoundation_stream_iterator).second.second.deviceIdentifier =
        g_value_get_string (&value);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
    (*iterator_2).second.second.deviceIdentifier = g_value_get_string (&value);
#endif
    g_value_unset (&value);
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (cb_data_p->fileDescriptor != -1)
    (*iterator_2).second.second.fileDescriptor = cb_data_p->fileDescriptor;
#endif

  //GtkFileChooserButton* file_chooser_button_p =
  //  GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  //ACE_ASSERT (file_chooser_button_p);
  //GFile* file_p =
  //  gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_button_p));
  //if (file_p)
  //{
  //  char* filename_p = g_file_get_path (file_p);
  //  if (!filename_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));

  //    // clean up
  //    g_object_unref (file_p);

  //    goto error;
  //  } // end IF
  //  g_object_unref (file_p);
  //  data_p->configuration->moduleHandlerConfiguration.targetFileName =
  //    Common_UI_GTK_Tools::UTF82Locale (filename_p, -1);
  //  g_free (filename_p);
  //} // end IF
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  value_d = gtk_spin_button_get_value (spin_button_p);
  ACE_ASSERT (value_d);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      directshow_cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
        static_cast<unsigned int> (value_d);
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      mediafoundation_cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
        static_cast<unsigned int> (value_d);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  cb_data_p->configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
    static_cast<unsigned int> (value_d);
#endif

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.format);
      //ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.session);

      // *NOTE*: reusing a media session doesn't work reliably at the moment
      //         --> recreate a new session every time
      if ((*mediafoundation_stream_iterator).second.second.session)
      {
        //HRESULT result = E_FAIL;
        // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
        //result =
        //  data_p->configuration->moduleHandlerConfiguration.session->Shutdown ();
        //if (FAILED (result))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        (*mediafoundation_stream_iterator).second.second.session->Release ();
        (*mediafoundation_stream_iterator).second.second.session = NULL;
      } // end IF

      ////if (!Stream_Module_Device_Tools::setCaptureFormat (data_p->configuration->moduleHandlerConfiguration.builder,
      //if (!Stream_Module_Device_Tools::setCaptureFormat (topology_p,

      //#if defined (ACE_WIN32) || defined (ACE_WIN64)
      //  topology_p->Release ();
      //#endif
      //struct _AMMediaType* media_type_p = NULL;
      //Stream_Module_Device_Tools::getCaptureFormat (data_p->configuration->moduleHandlerConfiguration.builder,
      //                                              media_type_p);
      //media_type.Set (*media_type_p);
      //ACE_ASSERT (media_type == *data_p->configuration->moduleHandlerConfiguration.format);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  if (!Stream_Module_Device_Tools::setFormat ((*iterator_2).second.second.fileDescriptor,
                                              (*iterator_2).second.second.inputFormat))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), aborting\n")));
    goto error;
  } // end IF
  if (!Stream_Module_Device_Tools::setFrameRate ((*iterator_2).second.second.fileDescriptor,
                                                 (*iterator_2).second.second.frameRate))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFrameRate(), aborting\n")));
    goto error;
  } // end IF
#endif

  // step3: start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    struct Test_U_GTK_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  thread_data_p->CBData = data_base_p;
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
//                  ACE_TEXT (TEST_U_Stream_CamSave_THREAD_NAME));
//  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_U_STREAM_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_base_p->lock);
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

      // clean up
//    delete thread_name_p;
      delete thread_data_p;

      goto error;
    } // end IF

    // step3: start progress reporting
    //ACE_ASSERT (!data_p->progressEventSourceId);
    data_base_p->progressEventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                            // _LOW doesn't work (on Win32)
                          COMMON_UI_GTK_INTERVAL_DEFAULT_PROGRESSBAR_REFRESH, // ms (?)
                          idle_update_progress_cb,
                          &data_base_p->progressData,
                          NULL);
    if (!data_base_p->progressEventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", returning\n")));

      // clean up
      ACE_THR_FUNC_RETURN exit_status;
      result = thread_manager_p->join (thread_id, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    thread_id));

      goto error;
    } // end IF
    thread_data_p->eventSourceId = data_base_p->progressEventSourceId;
    data_base_p->progressData.pendingActions[data_base_p->progressEventSourceId] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    data_base_p->eventSourceIds.insert (data_base_p->progressEventSourceId);
  } // end lock scope

  return;

error:
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in),
                           GTK_STOCK_MEDIA_RECORD);
  //gtk_action_set_sensitive (action_p, false);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
  //                          true);
} // toggleaction_record_toggled_cb

void
toggleaction_save_toggled_cb (GtkToggleAction* toggleAction_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_save_toggled_cb"));

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_base_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GError* error_p = NULL;
  GFile* file_p = NULL;
  std::string filename_string;
  if (gtk_toggle_action_get_active (toggleAction_in))
  {
    file_p =
      gtk_file_chooser_get_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p));
    ACE_ASSERT (file_p);
    char* filename_p = g_file_get_path (file_p);
    ACE_ASSERT (filename_p);

    filename_string = filename_p;

    g_free (filename_p);
    g_object_unref (G_OBJECT (file_p));
  } // end IF
  else
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
                  ACE_TEXT (file_p),
                  ACE_TEXT (error_p->message)));

      // clean up
      g_error_free (error_p);
      g_object_unref (file_p);

      return;
    } // end IF
    g_object_unref (file_p);
  } // end ELSE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      (*directshow_stream_iterator).second.second.targetFileName =
        filename_string;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      (*mediafoundation_stream_iterator).second.second.targetFileName =
        filename_string;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*iterator_2).second.second.targetFileName = filename_string;
#endif
} // toggleaction_save_toggled_cb

void
toggleaction_fullscreen_toggled_cb (GtkToggleAction* toggleAction_in,
                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_fullscreen_toggled_cb"));

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  bool is_active = gtk_toggle_action_get_active (toggleAction_in);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_base_p->builders.end ());

  Stream_IStreamControlBase* stream_base_p = NULL;
  Stream_IStream_t* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      stream_base_p = directshow_cb_data_p->stream;
      stream_p = directshow_cb_data_p->stream;
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      (*directshow_stream_iterator).second.second.fullScreen = is_active;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      stream_base_p = mediafoundation_cb_data_p->stream;
      stream_p = mediafoundation_cb_data_p->stream;
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      (*mediafoundation_stream_iterator).second.second.fullScreen = is_active;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  stream_base_p = cb_data_p->stream;
  stream_p = cb_data_p->stream;
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
  (*iterator_2).second.second.fullScreen = is_active;
#endif
  ACE_ASSERT (stream_base_p);
  if (!stream_base_p->isRunning ())
    return;

  ACE_ASSERT (iterator != data_base_p->builders.end ());
  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_WINDOW_FULLSCREEN)));
  ACE_ASSERT (window_p);

  if (is_active)
  {
    gtk_widget_show (GTK_WIDGET (window_p));
//  gtk_window_fullscreen (window_p);
    gtk_window_maximize (window_p);
  } // end IF
  else
  {
//    gtk_window_minimize (window_p);
//  gtk_window_unfullscreen (window_p);
    gtk_widget_hide (GTK_WIDGET (window_p));
  } // end ELSE

  ACE_ASSERT (stream_p);
  const Stream_Module_t* module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      module_p =
        stream_p->find (ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      module_p =
        stream_p->find (ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING));
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unkown media framework (was: %d), returning\n"),
                  ACE_TEXT (stream_p->name ().c_str ()),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  module_p =
      stream_p->find (ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_GTK_CAIRO_DEFAULT_NAME_STRING));
#endif
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IStream::find(\"Display\"), returning\n"),
                ACE_TEXT (stream_p->name ().c_str ())));
    return;
  } // end IF
  Stream_Module_Visualization_IFullscreen* ifullscreen_p =
    dynamic_cast<Stream_Module_Visualization_IFullscreen*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!ifullscreen_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:Display: failed to dynamic_cast<Stream_Module_Visualization_IFullscreen*>(0x%@), returning\n"),
                ACE_TEXT (stream_p->name ().c_str ()),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    return;
  } // end IF
  try {
    ifullscreen_p->toggle ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_Module_Visualization_IFullscreen::toggle(), returning\n")));
    return;
  }
} // toggleaction_fullscreen_toggled_cb

void
action_cut_activate_cb (GtkAction* action_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_cut_activate_cb"));

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Stream_CamSave_IStreamControl_t* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      stream_p = directshow_cb_data_p->stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      stream_p = mediafoundation_cb_data_p->stream;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  stream_p = cb_data_p->stream;
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

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);
} // action_report_activate_cb

// -----------------------------------------------------------------------------

gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_base_p->builders.end ());

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
  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_base_p->builders.end ());

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!dialog_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
    return TRUE; // propagate
  } // end IF

  // run dialog
  gint result = gtk_dialog_run (dialog_p);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  gtk_widget_hide (GTK_WIDGET (dialog_p));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Stream_CamSave_IStreamControl_t* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      stream_p = directshow_cb_data_p->stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      stream_p = mediafoundation_cb_data_p->stream;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return TRUE; // propagate
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  stream_p = cb_data_p->stream;
#endif
  ACE_ASSERT (stream_p);

  //// step1: remove event sources
  //{ ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->lock);
  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->eventSourceIds.begin ();
  //       iterator != data_p->eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->eventSourceIds.clear ();
  //} // end lock scope

  // stop stream ?
  const enum Stream_StateMachine_ControlState& status_r =
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

  struct Stream_CamSave_GTK_CBData* cb_data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

  Stream_IStream_t* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (cb_data_base_p);
      stream_p = directshow_cb_data_p->stream;
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (cb_data_base_p);
      stream_p = mediafoundation_cb_data_p->stream;
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (cb_data_base_p);
  stream_p = cb_data_p->stream;
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (stream_p);
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_3))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string device_identifier_string = g_value_get_string (&value);
  g_value_unset (&value);

  gint n_rows = 0;
  GtkToggleAction* toggle_action_p = NULL;
  bool result = false;

  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if ((*directshow_stream_iterator).second.second.builder)
      {
        (*directshow_stream_iterator).second.second.builder->Release ();
        (*directshow_stream_iterator).second.second.builder = NULL;
      } // end IF
      IAMBufferNegotiation* buffer_negotiation_p = NULL;
      if (directshow_cb_data_p->streamConfiguration)
      {
        directshow_cb_data_p->streamConfiguration->Release ();
        directshow_cb_data_p->streamConfiguration = NULL;
      } // end IF
      Stream_MediaFramework_DirectShow_Graph_t graph_layout;
      if (!Stream_Module_Device_DirectShow_Tools::loadDeviceGraph (device_identifier_string,
                                                                   CLSID_VideoInputDeviceCategory,
                                                                   (*directshow_stream_iterator).second.second.builder,
                                                                   buffer_negotiation_p,
                                                                   directshow_cb_data_p->streamConfiguration,
                                                                   graph_layout))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                    ACE_TEXT (device_identifier_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT ((*directshow_stream_iterator).second.second.builder);
      ACE_ASSERT (buffer_negotiation_p);
      ACE_ASSERT (directshow_cb_data_p->streamConfiguration);

      buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
      IMFMediaSourceEx* media_source_p = NULL;
#else
      IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (device_identifier_string,
                                                                       MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                       media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(\"%s\"), returning\n"),
                    ACE_TEXT (device_identifier_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (media_source_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

      std::string module_name =
        ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_RENDERER_NULL_MODULE_NAME);
      Stream_Module_t* module_p =
        const_cast<Stream_Module_t*> (stream_p->find (module_name));
      if (!module_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Base_T::find(\"%s\"), returning\n"),
                    ACE_TEXT (module_name.c_str ())));
        media_source_p->Release (); media_source_p = NULL;
        return;
      } // end IF
      Stream_CamSave_MediaFoundation_DisplayNull* display_impl_p =
        dynamic_cast<Stream_CamSave_MediaFoundation_DisplayNull*> (module_p->writer ());
      ACE_ASSERT (display_impl_p);

      IMFTopology* topology_p = NULL;
      struct _MFRatio pixel_aspect_ratio = { 1, 1 };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology (device_identifier_string,
                                                                           MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                           media_source_p,
                                                                           display_impl_p,
                                                                           topology_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology(), returning\n")));
        media_source_p->Release (); media_source_p = NULL;
        return;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      ACE_ASSERT (topology_p);
      media_source_p->Release (); media_source_p = NULL;

      // sanity check(s)
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.session);

      if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                     (*mediafoundation_stream_iterator).second.second.session,
                                                                     true,
                                                                     true))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), returning\n")));
        topology_p->Release (); topology_p = NULL;
        return;
      } // end IF
      topology_p->Release (); topology_p = NULL;

      if ((*mediafoundation_stream_iterator).second.second.inputFormat)
      {
        (*mediafoundation_stream_iterator).second.second.inputFormat->Release ();
        (*mediafoundation_stream_iterator).second.second.inputFormat = NULL;
      } // end IF
      HRESULT result_2 =
        MFCreateMediaType (&(*mediafoundation_stream_iterator).second.second.inputFormat);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.inputFormat);
      result_2 =
        (*mediafoundation_stream_iterator).second.second.inputFormat->SetGUID (MF_MT_MAJOR_TYPE,
                                                                               MFMediaType_Video);
      ACE_ASSERT (SUCCEEDED (result_2));
      result_2 =
        (*mediafoundation_stream_iterator).second.second.inputFormat->SetUINT32 (MF_MT_INTERLACE_MODE,
                                                                                 MFVideoInterlace_Unknown);
      ACE_ASSERT (SUCCEEDED (result_2));
      result_2 =
        MFSetAttributeRatio ((*mediafoundation_stream_iterator).second.second.inputFormat,
                             MF_MT_PIXEL_ASPECT_RATIO,
                             pixel_aspect_ratio.Numerator,
                             pixel_aspect_ratio.Denominator);
      ACE_ASSERT (SUCCEEDED (result_2));

      //if (_DEBUG)
      //{
      //  std::string log_file_name =
      //    Common_File_Tools::getLogDirectory (std::string (),
      //                                        0);
      //  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      //  log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
      //  Stream_Module_Device_Tools::debug (data_p->configuration->moduleHandlerConfiguration.builder,
      //                                     log_file_name);
      //} // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result = load_formats (directshow_cb_data_p->streamConfiguration,
                             list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
      IMFMediaSourceEx* media_source_p = NULL;
#else
      IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
      if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_stream_iterator).second.second.session,
                                                                        media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (media_source_p);

      //if (!load_formats (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result = load_formats (media_source_p,
                             list_store_p);
      media_source_p->Release (); media_source_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  int result_2 = -1;
  if (cb_data_p->fileDescriptor != -1)
  {
    result_2 = v4l2_close (cb_data_p->fileDescriptor);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  cb_data_p->fileDescriptor));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed v4l2 device (fd was: %d)...\n"),
                cb_data_p->fileDescriptor));
    cb_data_p->fileDescriptor = -1;
  } // end IF
  ACE_ASSERT (cb_data_p->fileDescriptor == -1);
  int open_mode =
      (((*iterator_2).second.second.v4l2Method == V4L2_MEMORY_MMAP) ? O_RDWR
                                                                    : O_RDONLY);
  cb_data_p->fileDescriptor = v4l2_open (device_identifier_string.c_str (),
                                         open_mode);
  if (cb_data_p->fileDescriptor == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", returning\n"),
                ACE_TEXT (device_identifier_string.c_str ()), open_mode));
    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened v4l2 device \"%s\" (fd: %d)\n"),
              ACE_TEXT (device_identifier_string.c_str ()),
              cb_data_p->fileDescriptor));

  result = load_formats (cb_data_p->fileDescriptor,
                         list_store_p);
#endif
  if (!result)
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
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF

  toggle_action_p =
      GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), TRUE);
} // combobox_source_changed_cb

void
combobox_format_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  struct Stream_CamSave_GTK_CBData* cb_data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (cb_data_base_p);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (cb_data_base_p);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (cb_data_base_p);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_3))
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
#endif
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);

  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      ACE_ASSERT (directshow_cb_data_p->streamConfiguration);
      //ACE_ASSERT ((*directshow_stream_iterator).second.second.builder);
      //ACE_ASSERT ((*directshow_stream_iterator).second.second.inputFormat);

      //struct _AMMediaType* media_type_p = NULL;
      //result_2 =
      //  directshow_cb_data_p->streamConfiguration->GetFormat (&media_type_p);
      //if (FAILED (result_2))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      //  return;
      //} // end IF
      //ACE_ASSERT (media_type_p);
      //media_type_p->subtype = GUID_s;
      //(*directshow_stream_iterator).second.second.inputFormat->subtype = GUID_s;
      //FOURCCMap fourcc_map (&GUID_s);
      ////if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
      //if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo))
      //{
      //  struct tagVIDEOINFOHEADER* video_info_header_p =
      //    //(struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      //    (struct tagVIDEOINFOHEADER*)(*directshow_stream_iterator).second.second.inputFormat->pbFormat;
      //  video_info_header_p->bmiHeader.biCompression =
      //    (Stream_Module_Decoder_Tools::isCompressedVideo (GUID_s,
      //                                                     STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ? fourcc_map.GetFOURCC ()
      //                                                                                       : 0);
      //} // end IF
      ////else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
      //else if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo2))
      //{
      //  // *NOTE*: these media subtypes do not work with the Video Renderer
      //  //         directly --> insert the Overlay Mixer
      //  struct tagVIDEOINFOHEADER2* video_info_header2_p =
      //    //(struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      //    (struct tagVIDEOINFOHEADER2*)(*directshow_stream_iterator).second.second.inputFormat->pbFormat;
      //  video_info_header2_p->bmiHeader.biCompression =
      //    (Stream_Module_Decoder_Tools::isCompressedVideo (GUID_s,
      //                                                     STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ? fourcc_map.GetFOURCC ()
      //                                                                                       : 0);
      //} // end ELSE IF
      //else
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
      //              ACE_TEXT (Common_Tools::GUIDToString ((*directshow_stream_iterator).second.second.inputFormat->formattype).c_str ())));
      //  goto error;
      //} // end ELSE

      // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
      //         the filter pins are associated. IGraphConfig::Reconnect fails
      //         unless the graph is "disconnected" first
      //if (!Stream_MediaFramework_DirectShow_Tools::disconnect ((*directshow_stream_iterator).second.second.builder))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), returning\n")));
      //  goto error;
      //} // end IF
      //if (!Stream_Module_Device_DirectShow_Tools::setCaptureFormat ((*directshow_stream_iterator).second.second.builder,
      //                                                              CLSID_VideoInputDeviceCategory,
      //                                                              *(*directshow_stream_iterator).second.second.inputFormat))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::setCaptureFormat(), returning\n")));
      //  goto error;
      //} // end IF
      //Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      //goto continue_;

//error:
//      //Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
//
//      return;
//
//continue_:
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.inputFormat);
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.session);

      result_2 =
        (*mediafoundation_stream_iterator).second.second.inputFormat->SetGUID (MF_MT_SUBTYPE,
                                                                               GUID_s);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF

      IMFTopology* topology_p = NULL;
      if (!Stream_MediaFramework_MediaFoundation_Tools::getTopology ((*mediafoundation_stream_iterator).second.second.session,
                                                                     topology_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getTopology(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (topology_p);
      if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                         (*mediafoundation_stream_iterator).second.second.inputFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), returning\n")));
        topology_p->Release (); topology_p = NULL;
        return;
      } // end IF
      topology_p->Release (); topology_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
//  (*iterator_2).second.second.inputFormat =
//      Stream_Module_Device_Tools::v4l2FormatToffmpegFormat (format_i);
  (*iterator_2).second.second.inputFormat.fmt.pix.pixelformat = format_i;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result = load_resolutions (directshow_cb_data_p->streamConfiguration,
                                 GUID_s,
                                 list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
      IMFMediaSourceEx* media_source_p = NULL;
#else
      IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
      if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_stream_iterator).second.second.session,
                                                                        media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (media_source_p);

      //if (!load_resolutions (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result = load_resolutions (media_source_p,
                                 GUID_s,
                                 list_store_p);
      media_source_p->Release (); media_source_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  result = load_resolutions (cb_data_p->fileDescriptor,
                             format_i,
                             list_store_p);
#endif
  if (!result)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_resolutions(\"%s\"), returning\n"),
                Stream_MediaFramework_Tools::mediaSubTypeToString (GUID_s, cb_data_base_p->mediaFramework).c_str ()));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_resolutions(%d,%u), returning\n"),
                cb_data_p->fileDescriptor,
                format_i));
#endif
    return;
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
} // combobox_format_changed_cb

void
combobox_resolution_changed_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  struct Stream_CamSave_GTK_CBData* cb_data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (cb_data_base_p);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (cb_data_base_p);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (cb_data_base_p);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

  GtkTreeIter iterator_3;
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_3))
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
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_3,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s =
    Common_Tools::StringToGUID (g_value_get_string (&value));
  ACE_ASSERT (!InlineIsEqualGUID (GUID_s, GUID_NULL));
#else
  __u32 format_i = 0;
  std::istringstream converter;
  converter.str (g_value_get_string (&value));
  converter >> format_i;
#endif
  g_value_unset (&value);
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_3))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
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
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);

    bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      ACE_ASSERT (directshow_cb_data_p->streamConfiguration);
      //ACE_ASSERT ((*directshow_stream_iterator).second.second.builder);
      //ACE_ASSERT ((*directshow_stream_iterator).second.second.inputFormat);

      //struct _AMMediaType* media_type_p = NULL;
      //result_2 =
      //  directshow_cb_data_p->streamConfiguration->GetFormat (&media_type_p);
      //if (FAILED (result_2))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      //  return;
      //} // end IF
      //ACE_ASSERT (media_type_p);
      //if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
      //if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo))
      //{
      //  struct tagVIDEOINFOHEADER* video_info_header_p =
      //    //(struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      //    (struct tagVIDEOINFOHEADER*)(*directshow_stream_iterator).second.second.inputFormat->pbFormat;
      //  video_info_header_p->bmiHeader.biWidth = width;
      //  video_info_header_p->bmiHeader.biHeight = height;
      //} // end IF
      ////else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
      //else if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo2))
      //{
      //  // *NOTE*: these media subtypes do not work with the Video Renderer
      //  //         directly --> insert the Overlay Mixer
      //  struct tagVIDEOINFOHEADER2* video_info_header2_p =
      //    //(struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      //    (struct tagVIDEOINFOHEADER2*)(*directshow_stream_iterator).second.second.inputFormat->pbFormat;
      //  video_info_header2_p->bmiHeader.biWidth = width;
      //  video_info_header2_p->bmiHeader.biHeight = height;
      //} // end ELSE IF
      //else
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
      //              ACE_TEXT (Common_Tools::GUIDToString ((*directshow_stream_iterator).second.second.inputFormat->formattype).c_str ())));
      //  goto error;
      //} // end ELSE

      // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
      //         the filter pins are associated. IGraphConfig::Reconnect fails
      //         unless the graph is "disconnected" first
      //if (!Stream_MediaFramework_DirectShow_Tools::disconnect ((*directshow_stream_iterator).second.second.builder))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), returning\n")));
      //  goto error;
      //} // end IF
      //if (!Stream_Module_Device_DirectShow_Tools::setCaptureFormat ((*directshow_stream_iterator).second.second.builder,
      //                                                              CLSID_VideoInputDeviceCategory,
      //                                                              *(*directshow_stream_iterator).second.second.inputFormat))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::setCaptureFormat(), returning\n")));
      //  goto error;
      //} // end IF
      ////Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      //goto continue_;

//error:
//      //Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
//
//      return;
//
//continue_:
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.inputFormat);
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.session);

      result_2 =
        (*mediafoundation_stream_iterator).second.second.inputFormat->SetUINT32 (MF_MT_SAMPLE_SIZE,
                                                                                 width * height * 3);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_SAMPLE_SIZE,%d): \"%s\", returning\n"),
                    width * height * 3,
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF
      result_2 =
        MFSetAttributeSize ((*mediafoundation_stream_iterator).second.second.inputFormat,
                            MF_MT_FRAME_SIZE,
                            width, height);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFSetAttributeSize(%d,%d): \"%s\", returning\n"),
                    width, height,
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF

      IMFTopology* topology_p = NULL;
      if (!Stream_MediaFramework_MediaFoundation_Tools::getTopology ((*mediafoundation_stream_iterator).second.second.session,
                                                                     topology_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getTopology(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (topology_p);
      if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                         (*mediafoundation_stream_iterator).second.second.inputFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), returning\n")));
        topology_p->Release (); topology_p = NULL;
        return;
      } // end IF
      topology_p->Release (); topology_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*iterator_2).second.second.sourceFormat.height = height;
  (*iterator_2).second.second.sourceFormat.width = width;
  (*iterator_2).second.second.inputFormat.fmt.pix.height = height;
  (*iterator_2).second.second.inputFormat.fmt.pix.width = width;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result = load_rates (directshow_cb_data_p->streamConfiguration,
                           GUID_s,
                           width,
                           list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
      IMFMediaSourceEx* media_source_p = NULL;
#else
      IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
      if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_stream_iterator).second.second.session,
                                                                        media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (media_source_p);

      //if (!load_rates (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result = load_rates (media_source_p,
                           GUID_s,
                           width,
                           list_store_p);
      media_source_p->Release (); media_source_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  result = load_rates (cb_data_p->fileDescriptor,
                       format_i,
                       width, height,
                       list_store_p);
#endif
  if (!result)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_rates(%s,%u), returning\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (GUID_s, cb_data_base_p->mediaFramework).c_str ()),
                width));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_rates(%d,%u,%u,%u), returning\n"),
                cb_data_p->fileDescriptor,
                format_i,
                width, height));
#endif
    return;
  } // end IF

  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (!!n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RATE_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
} // combobox_resolution_changed_cb

void
combobox_rate_changed_cb (GtkWidget* widget_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_rate_changed_cb"));

  struct Stream_CamSave_GTK_CBData* cb_data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (cb_data_base_p);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (cb_data_base_p);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (cb_data_base_p);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (iterator != cb_data_base_p->builders.end ());

  GtkTreeIter iterator_3;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_3))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value;
  g_value_init (&value, G_TYPE_UINT);
  GValue value_2;
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
  unsigned int frame_interval = g_value_get_uint (&value);
  g_value_unset (&value);
  unsigned int frame_interval_denominator = g_value_get_uint (&value_2);
  g_value_unset (&value_2);

  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  switch (cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      ACE_ASSERT (directshow_cb_data_p->streamConfiguration);
      //ACE_ASSERT ((*directshow_stream_iterator).second.second.builder);
      //ACE_ASSERT ((*directshow_stream_iterator).second.second.inputFormat);

      //struct _AMMediaType* media_type_p = NULL;
      //result_2 =
      //  directshow_cb_data_p->streamConfiguration->GetFormat (&media_type_p);
      //if (FAILED (result_2))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      //  return;
      //} // end IF
      //ACE_ASSERT (media_type_p);

      //if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
      //if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo))
      //{
      //  struct tagVIDEOINFOHEADER* video_info_header_p =
      //    //(struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      //    (struct tagVIDEOINFOHEADER*)(*directshow_stream_iterator).second.second.inputFormat->pbFormat;
      //    video_info_header_p->AvgTimePerFrame = (10000000 / frame_interval);
      //} // end IF
      ////else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
      //else if (InlineIsEqualGUID ((*directshow_stream_iterator).second.second.inputFormat->formattype, FORMAT_VideoInfo2))
      //{
      //  // *NOTE*: these media subtypes do not work with the Video Renderer
      //  //         directly --> insert the Overlay Mixer
      //  struct tagVIDEOINFOHEADER2* video_info_header2_p =
      //    //(struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      //    (struct tagVIDEOINFOHEADER2*)(*directshow_stream_iterator).second.second.inputFormat->pbFormat;
      //  video_info_header2_p->AvgTimePerFrame = (10000000 / frame_interval);
      //} // end ELSE IF
      //else
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
      //              ACE_TEXT (Common_Tools::GUIDToString ((*directshow_stream_iterator).second.second.inputFormat->formattype).c_str ())));
      //  goto error;
      //} // end ELSE

      // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
      //         the filter pins are associated. IGraphConfig::Reconnect fails
      //         unless the graph is "disconnected" first
//      if (!Stream_MediaFramework_DirectShow_Tools::disconnect ((*directshow_stream_iterator).second.second.builder))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), returning\n")));
//        goto error;
//      } // end IF
//      if (!Stream_Module_Device_DirectShow_Tools::setCaptureFormat ((*directshow_stream_iterator).second.second.builder,
//                                                                    CLSID_VideoInputDeviceCategory,
//                                                                    *(*directshow_stream_iterator).second.second.inputFormat))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::setCaptureFormat(), returning\n")));
//        goto error;
//      } // end IF
//      //Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
//      goto continue_;
//
//error:
//      //Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
//
//      return;
//
//continue_:
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ACE_UNUSED_ARG (frame_interval_denominator);

      // sanity check(s)
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second.inputFormat);

      UINT32 width, height;
      result_2 =
        MFGetAttributeSize ((*mediafoundation_stream_iterator).second.second.inputFormat,
                            MF_MT_FRAME_SIZE,
                            &width, &height);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFGetAttributeSize(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF

      UINT32 bit_rate = width * height;
      bit_rate *=
        static_cast<UINT32> (((double)frame_interval / (double)frame_interval_denominator) * 3 * 8);
      result_2 =
        (*mediafoundation_stream_iterator).second.second.inputFormat->SetUINT32 (MF_MT_AVG_BITRATE,
                                                                                 bit_rate);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_AVG_BITRATE,%u): \"%s\", returning\n"),
                    bit_rate,
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF
      result_2 =
        MFSetAttributeSize ((*mediafoundation_stream_iterator).second.second.inputFormat,
                            MF_MT_FRAME_RATE,
                            frame_interval, frame_interval_denominator);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFSetAttributeSize(MF_MT_FRAME_RATE,%f): \"%s\", returning\n"),
                    (float)frame_interval / (float)frame_interval_denominator,
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        return;
      } // end IF

      IMFTopology* topology_p = NULL;
      if (!Stream_MediaFramework_MediaFoundation_Tools::getTopology ((*mediafoundation_stream_iterator).second.second.session,
                                                                     topology_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getTopology(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (topology_p);
      if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                         (*mediafoundation_stream_iterator).second.second.inputFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), returning\n")));
        topology_p->Release (); topology_p = NULL;
        return;
      } // end IF
      topology_p->Release (); topology_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // *NOTE*: the frame rate is the reciprocal value of the time-per-frame
  //         interval
  (*iterator_2).second.second.frameRate.numerator =
    frame_interval_denominator;
  (*iterator_2).second.second.frameRate.denominator = frame_interval;
#endif
  set_capture_format (cb_data_base_p);
  update_buffer_size (cb_data_base_p);
} // combobox_rate_changed_cb

gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  // sanity check(s)
  ACE_UNUSED_ARG (widget_in);
  ACE_ASSERT (userData_in);

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);

  // sanity check(s)
  ACE_ASSERT (cb_data_p->pixelBufferLock);
  ACE_ASSERT (context_in);
  if (!cb_data_p->pixelBuffer)
    return TRUE; // --> widget has not been realized yet

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *cb_data_p->pixelBufferLock, FALSE);
    gdk_cairo_set_source_pixbuf (context_in,
                                 cb_data_p->pixelBuffer,
                                 0.0, 0.0);

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  // *NOTE*: media foundation capture frames are v-flipped
//  cairo_rotate (context_p, 180.0 * M_PI / 180.0);
//#endif
    cairo_paint (context_in);
  } // end lock scope
#endif

  return TRUE;
}

//void
//drawingarea_configure_event_cb (GtkWindow* window_in,
//                                GdkEvent* event_in,
//                                gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

//  Stream_CamSave_GTK_CBData* data_p =
//    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

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

//  Common_UI_GTK_BuildersIterator_t iterator =
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
//  //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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

  // sanity check(s)
  ACE_ASSERT (widget_in);

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  // sanity check(s)
  if (!window_p)
    return; // window is not (yet) realized, nothing to do
  if (!gdk_window_is_viewable (window_p))
    return; // window is not (yet) mapped, nothing to do

  // sanity check(s)
  ACE_ASSERT (allocation_in);
  ACE_ASSERT (userData_in);

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_base_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (iterator != data_base_p->builders.end ());

  //GtkToggleAction* toggle_action_p =
  //    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
  //                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_FULLSCREEN_NAME)));
  //ACE_ASSERT (toggle_action_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result = E_FAIL;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //// sanity check(s)
      //ACE_ASSERT (data_base_p->pixelBuffer == (*directshow_stream_iterator).second.second.pixelBuffer);
      //ACE_ASSERT (data_base_p->pixelBufferLock == (*directshow_stream_iterator).second.second.pixelBufferLock);

      (*directshow_stream_iterator).second.second.area.bottom =
        allocation_in->y + allocation_in->height;
      (*directshow_stream_iterator).second.second.area.left = allocation_in->x;
      (*directshow_stream_iterator).second.second.area.right =
        allocation_in->x + allocation_in->width;
      (*directshow_stream_iterator).second.second.area.top = allocation_in->y;

      if ((*directshow_stream_iterator).second.second.windowController)
        result =
          (*directshow_stream_iterator).second.second.windowController->SetWindowPosition ((*directshow_stream_iterator).second.second.area.left,
                                                                                           (*directshow_stream_iterator).second.second.area.top,
                                                                                           ((*directshow_stream_iterator).second.second.area.right -
                                                                                            (*directshow_stream_iterator).second.second.area.left),
                                                                                           ((*directshow_stream_iterator).second.second.area.bottom -
                                                                                            (*directshow_stream_iterator).second.second.area.top));
      else if ((*directshow_stream_iterator).second.second.windowController2)
      {
        struct tagRECT destination_rectangle_s;
        result = SetRectEmpty (&destination_rectangle_s);
        ACE_ASSERT (SUCCEEDED (result));
        destination_rectangle_s.right =
          ((*directshow_stream_iterator).second.second.area.right -
           (*directshow_stream_iterator).second.second.area.left);
        destination_rectangle_s.bottom =
          ((*directshow_stream_iterator).second.second.area.bottom -
           (*directshow_stream_iterator).second.second.area.top);
        result =
          (*directshow_stream_iterator).second.second.windowController2->SetVideoPosition (NULL,                      // <-- default: entire video
                                                                                           &destination_rectangle_s); // <-- entire window
      } // end ELSE IF
      else
        break;
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
                    (*directshow_stream_iterator).second.second.area.left, (*directshow_stream_iterator).second.second.area.top,
                    (*directshow_stream_iterator).second.second.area.right, (*directshow_stream_iterator).second.second.area.bottom,
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //// sanity check(s)
      //ACE_ASSERT (data_base_p->pixelBuffer == (*mediafoundation_stream_iterator).second.second.pixelBuffer);
      //ACE_ASSERT (data_base_p->pixelBufferLock == (*mediafoundation_stream_iterator).second.second.pixelBufferLock);

      (*mediafoundation_stream_iterator).second.second.area.bottom =
        allocation_in->y + allocation_in->height;
      (*mediafoundation_stream_iterator).second.second.area.left =
        allocation_in->x;
      (*mediafoundation_stream_iterator).second.second.area.right =
        allocation_in->x + allocation_in->width;
      (*mediafoundation_stream_iterator).second.second.area.top =
        allocation_in->y;

      if (!(*mediafoundation_stream_iterator).second.second.windowController)
        break;
      struct tagRECT destination_rectangle_s;
      result = SetRectEmpty (&destination_rectangle_s);
      ACE_ASSERT (SUCCEEDED (result));
      destination_rectangle_s.right =
        ((*mediafoundation_stream_iterator).second.second.area.right -
         (*mediafoundation_stream_iterator).second.second.area.left);
      destination_rectangle_s.bottom =
        ((*mediafoundation_stream_iterator).second.second.area.bottom -
         (*mediafoundation_stream_iterator).second.second.area.top);
      result =
        (*mediafoundation_stream_iterator).second.second.windowController->SetVideoPosition (NULL,                      // <-- default: entire video
                                                                                             &destination_rectangle_s); // <-- entire window
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFVideoDisplayControl::SetVideoPosition (NULL,%d,%d,%d,%d): \"%s\", continuing\n"),
                    destination_rectangle_s.left, destination_rectangle_s.top,
                    destination_rectangle_s.right, destination_rectangle_s.bottom,
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_ASSERT (cb_data_p->pixelBuffer == (*iterator_2).second.second.pixelBuffer);
  ACE_ASSERT (cb_data_p->pixelBufferLock == (*iterator_2).second.second.pixelBufferLock);
  ACE_ASSERT (cb_data_p->pixelBufferLock);

  (*iterator_2).second.second.area = *allocation_in;

#if GTK_CHECK_VERSION (3,0,0)
#else
  GdkPixbuf* pixbuf_p =
    gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                    TRUE,
                    8,
                    allocation_in->width, allocation_in->height);
  if (!pixbuf_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_new(), returning\n")));
    return;
  } // end IF
#endif

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *cb_data_p->pixelBufferLock);
    if (cb_data_p->pixelBuffer)
    {
      g_object_unref (cb_data_p->pixelBuffer);
      cb_data_p->pixelBuffer = NULL;
    } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    switch (data_base_p->mediaFramework)
//    {
//      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//        (*directshow_stream_iterator).second.second.pixelBuffer = NULL;
//        break;
//      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//        (*mediafoundation_stream_iterator).second.second.pixelBuffer = NULL;
//        break;
//      default:
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                    data_base_p->mediaFramework));
//        return;
//      }
//    } // end SWITCH
//#else
    (*iterator_2).second.second.pixelBuffer = NULL;
//#endif
    cb_data_p->pixelBuffer =
#if GTK_CHECK_VERSION (3,0,0)
      gdk_pixbuf_get_from_window (window_p,
                                  0, 0,
                                  allocation_in->width, allocation_in->height);
#else
      gdk_pixbuf_get_from_drawable (pixbuf_p,
                                    GDK_DRAWABLE (window_p),
                                    NULL,
                                    0, 0,
                                    0, 0, allocation_in->width, allocation_in->height);
#endif
    if (!cb_data_p->pixelBuffer)
    {
#if GTK_CHECK_VERSION (3,0,0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gdk_pixbuf_get_from_window(%@), returning\n"),
                  window_p));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gdk_pixbuf_get_from_drawable(%@), returning\n"),
                  GDK_DRAWABLE (window_p)));
      gdk_pixbuf_unref (pixbuf_p);
#endif
      return;
    } // end IF

    //    GHashTable* hash_table_p = gdk_pixbuf_get_options (cb_data_p->pixelBuffer);
    //    GHashTableIter iterator;
    //    g_hash_table_iter_init (&iterator, hash_table_p);
    //    gpointer key, value;
    //    for (unsigned int i = 0;
    //         g_hash_table_iter_next (iterator, &key, &value);
    //         ++i)
    //      ACE_DEBUG ((LM_DEBUG,
    //                  ACE_TEXT ("%u: \"\" --> \"\"\n"),
    //                  i,
    //                  static_cast<gchar*> (key),
    //                  static_cast<gchar*> (value)));

    // sanity check(s)
    ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (cb_data_p->pixelBuffer) == 8);
    ACE_ASSERT (gdk_pixbuf_get_colorspace (cb_data_p->pixelBuffer) == GDK_COLORSPACE_RGB);
//    ACE_ASSERT (gdk_pixbuf_get_n_channels (data_base_p->pixelBuffer) == 3);

//    if (!gdk_pixbuf_get_has_alpha (data_p->pixelBuffer))
//    { ACE_ASSERT (gdk_pixbuf_get_n_channels (data_p->pixelBuffer) == 3);
//      GdkPixbuf* pixbuf_p =
//          gdk_pixbuf_add_alpha (data_p->pixelBuffer,
//                                FALSE, 0, 0, 0);
//      ACE_ASSERT (pixbuf_p);
//      gdk_pixbuf_unref (data_p->pixelBuffer);
//      data_p->pixelBuffer = pixbuf_p;
//    } // end IF
//    // sanity check(s)
//    ACE_ASSERT (gdk_pixbuf_get_has_alpha (data_p->pixelBuffer));
    ACE_ASSERT (gdk_pixbuf_get_n_channels (cb_data_p->pixelBuffer) == 4);

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    switch (data_base_p->mediaFramework)
//    {
//      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//        (*directshow_stream_iterator).second.second.pixelBuffer =
//          data_base_p->pixelBuffer;
//        break;
//      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//        (*mediafoundation_stream_iterator).second.second.pixelBuffer =
//          data_base_p->pixelBuffer;
//        break;
//      default:
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                    data_base_p->mediaFramework));
//        return;
//      }
//    } // end SWITCH
//#else
    (*iterator_2).second.second.pixelBuffer = cb_data_p->pixelBuffer;
//#endif
  } // end lock scope
#endif
} // drawingarea_size_allocate_cb

void
filechooserbutton_cb (GtkFileChooserButton* fileChooserButton_in,
                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_cb"));

  // sanity check(s)
  ACE_ASSERT (fileChooserButton_in);
  ACE_ASSERT (userData_in);

  struct Stream_CamSave_GTK_CBData* data_base_p =
    static_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_base_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_GTK_CBData* directshow_cb_data_p = NULL;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  struct Stream_CamSave_MediaFoundation_GTK_CBData* mediafoundation_cb_data_p =
    NULL;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_cb_data_p =
        static_cast<struct Stream_CamSave_DirectShow_GTK_CBData*> (data_base_p);
      ACE_ASSERT (directshow_cb_data_p->configuration);
      directshow_stream_iterator =
        directshow_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directshow_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_cb_data_p =
        static_cast<struct Stream_CamSave_MediaFoundation_GTK_CBData*> (data_base_p);
      ACE_ASSERT (mediafoundation_cb_data_p->configuration);
      mediafoundation_stream_iterator =
        mediafoundation_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediafoundation_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_V4L_GTK_CBData* cb_data_p =
    static_cast<struct Stream_CamSave_V4L_GTK_CBData*> (data_base_p);
  ACE_ASSERT (cb_data_p->configuration);
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T iterator_2 =
    cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (iterator != data_base_p->builders.end ());

  //// step1: display chooser dialog
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_FILECHOOSER_OPEN_NAME)));
  //ACE_ASSERT (file_chooser_dialog_p);

  GFile* file_p = NULL;
  char* string_p = NULL;
  gchar* string_2 = NULL;
  //// run dialog
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

  file_p =
      gtk_file_chooser_get_file (GTK_FILE_CHOOSER (fileChooserButton_in));
  ACE_ASSERT (file_p);
  string_p = g_file_get_path (file_p);
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

  std::string filename_string =
    Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
  if (filename_string.empty ())
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
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      (*directshow_stream_iterator).second.second.targetFileName =
        filename_string;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      (*mediafoundation_stream_iterator).second.second.targetFileName =
        filename_string;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*iterator_2).second.second.targetFileName = filename_string;
#endif

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ENTRY_DESTINATION_NAME)));
  ACE_ASSERT (entry_p);
  string_2 = Common_UI_GTK_Tools::localeToUTF8 (filename_string);
  ACE_ASSERT (string_2);
  gtk_entry_set_text (entry_p,
                      string_2);
  g_free (string_2);
} // filechooserbutton_cb

void
filechooserdialog_cb (GtkFileChooser* fileChooser_in,
                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserdialog_cb"));

  ACE_UNUSED_ARG (userData_in);

  gtk_dialog_response (GTK_DIALOG (GTK_FILE_CHOOSER_DIALOG (fileChooser_in)),
                       GTK_RESPONSE_ACCEPT);
} // filechooserdialog_cb

gboolean
key_cb (GtkWidget* widget_in,
        GdkEventKey* eventKey_in,
        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::key_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (eventKey_in);

  struct Stream_CamSave_GTK_CBData* data_base_p =
      reinterpret_cast<struct Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_base_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_base_p->builders.end ());

  switch (eventKey_in->keyval)
  {
#if GTK_CHECK_VERSION(3,0,0)
    case GDK_KEY_Escape:
    case GDK_KEY_f:
    case GDK_KEY_F:
#else
    case GDK_Escape:
    case GDK_f:
    case GDK_F:
#endif // GTK_CHECK_VERSION(3,0,0)
    {
      GtkToggleAction* toggle_action_p =
        GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_FULLSCREEN_NAME)));
      ACE_ASSERT (toggle_action_p);

      bool is_active = gtk_toggle_action_get_active (toggle_action_p);

      // sanity check(s)
      if ((eventKey_in->keyval == GDK_KEY_Escape) &&
          !is_active)
        break; // <-- not in fullscreen mode, nothing to do

      gtk_toggle_action_set_active (toggle_action_p,
                                    !is_active);

      break;
    }
    default:
      return FALSE; // propagate
  } // end SWITCH

  return TRUE; // done (do not propagate further)
}
gboolean
drawingarea_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey* eventKey_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_key_press_event_cb"));

  return key_cb (widget_in, eventKey_in, userData_in);
};
gboolean
dialog_main_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey* eventKey_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::dialog_main_key_press_event_cb"));

  return key_cb (widget_in, eventKey_in, userData_in);
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
