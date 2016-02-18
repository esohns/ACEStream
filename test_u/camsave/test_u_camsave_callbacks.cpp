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

#include "test_u_camsave_callbacks.h"

#include <limits>
#include <map>
#include <set>
#include <sstream>

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dshow.h"
#include "Dvdmedia.h"
//#include "streams.h"

#include "gdk/gdkwin32.h"
#else
#include "ace/Dirent_Selector.h"

#include "libv4l2.h"
#include "linux/videodev2.h"
#endif

#include "common_timer_manager.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_tools.h"

#include "stream_macros.h"

#include "stream_dev_defines.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_defines.h"
#include "test_u_camsave_stream.h"

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
load_capture_devices (GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_capture_devices"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  ICreateDevEnum* enumerator_p = NULL;
  result_2 =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);

  IEnumMoniker* enum_moniker_p = NULL;
  result_2 =
    enumerator_p->CreateClassEnumerator (CLSID_VideoInputDeviceCategory,
                                         &enum_moniker_p,
                                         0);
  if (result_2 != S_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(CLSID_VideoInputDeviceCategory): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    //result_2 = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    goto error;
  } // end IF
  ACE_ASSERT (enum_moniker_p);

  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  VARIANT variant;
  GtkTreeIter iterator;
  while (enum_moniker_p->Next (1, &moniker_p, NULL) == S_OK)
  {
    ACE_ASSERT (moniker_p);

    properties_p = NULL;
    result = moniker_p->BindToStorage (0, 0, IID_PPV_ARGS (&properties_p));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (properties_p);

    VariantInit (&variant);
    result_2 = properties_p->Read (L"FriendlyName", &variant, 0);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyBag::Read(Description/FriendlyName): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    properties_p->Release ();
    properties_p = NULL;
    ACE_Wide_To_Ascii converter (variant.bstrVal);
    VariantClear (&variant);
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.char_rep (),
                        -1);

    moniker_p->Release ();
    moniker_p = NULL;
  } // end WHILE
  result = true;

error:
  if (properties_p)
    properties_p->Release ();
  if (moniker_p)
    moniker_p->Release ();
  if (enum_moniker_p)
    enum_moniker_p->Release ();
  if (enumerator_p)
    enumerator_p->Release ();
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
  GtkTreeIter iterator;
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

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ACE_TEXT (device_capabilities.card),
                        1, ACE_TEXT (device_filename.c_str ()),
                        -1);

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

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct less_guid
{
  bool operator() (const GUID& lhs_in, const GUID& rhs_in) const
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

  HRESULT result;
  int count = 0, size = 0;
  std::set<GUID, less_guid> GUIDs;
  std::string media_subtype_string;
  std::string GUID_stdstring;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  AM_MEDIA_TYPE* media_type_p = NULL;
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
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if ((media_type_p->formattype != FORMAT_VideoInfo) &&
        (media_type_p->formattype != FORMAT_VideoInfo2))
    {
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF

    // *NOTE*: FORMAT_VideoInfo2 types do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    GUIDs.insert (media_type_p->subtype);

    //DeleteMediaType (media_type_p);
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  OLECHAR GUID_string[39];
  ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
  for (std::set<GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    count = StringFromGUID2 (*iterator_2,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);
    ACE_Wide_To_Ascii converter (GUID_string);
    GUID_stdstring = converter.char_rep ();
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
load_resolutions (IAMStreamConfig* IAMStreamConfig_in,
                  const GUID& mediaSubType_in,
                  GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_resolutions"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result;
  int count = 0, size = 0;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  AM_MEDIA_TYPE* media_type_p = NULL;
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
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (media_type_p->subtype != mediaSubType_in)
    {
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    if (media_type_p->formattype == FORMAT_VideoInfo)
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      resolutions.insert (std::make_pair (video_info_header_p->bmiHeader.biWidth,
                                          video_info_header_p->bmiHeader.biHeight));
    } // end IF
    else if (media_type_p->formattype == FORMAT_VideoInfo2)
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
                  ACE_TEXT ("invalid AM_MEDIA_TYPE, aborting\n")));

      // clean up
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);

      return false;
    } // end ELSE
    //DeleteMediaType (media_type_p);
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
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
load_rates (IAMStreamConfig* IAMStreamConfig_in,
            const GUID& mediaSubType_in,
            unsigned int width_in,
            GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_rates"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result;
  int count = 0, size = 0;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  AM_MEDIA_TYPE* media_type_p = NULL;
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
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (media_type_p->subtype != mediaSubType_in)
    {
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    if (media_type_p->formattype == FORMAT_VideoInfo)
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      if (video_info_header_p->bmiHeader.biWidth != width_in)
      {
        //DeleteMediaType (media_type_p);
        Stream_Module_Device_Tools::deleteMediaType (media_type_p);
        continue;
      } // end IF
      else
        frame_duration =
          static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end IF
    else if (media_type_p->formattype == FORMAT_VideoInfo2)
    {
      video_info_header2_p = (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      if (video_info_header2_p->bmiHeader.biWidth != width_in)
      {
        //DeleteMediaType (media_type_p);
        Stream_Module_Device_Tools::deleteMediaType (media_type_p);
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

      // clean up
      //DeleteMediaType (media_type_p);
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);

      return false;
    } // end IF
    frame_rates.insert (std::make_pair (10000000 / static_cast<unsigned int> (capabilities.MinFrameInterval),
                                        frame_duration));
    //DeleteMediaType (media_type_p);
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end WHILE
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
  std::set<v4l2_fract, less_fract> frame_intervals;
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

/////////////////////////////////////////

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif

  Stream_CamSave_ThreadData* data_p =
      static_cast<Stream_CamSave_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->CBData);
  ACE_ASSERT (data_p->CBData->configuration);
  ACE_ASSERT (data_p->CBData->stream);

  const Stream_CamSave_SessionData_t* session_data_container_p = NULL;
  const Stream_CamSave_SessionData* session_data_p = NULL;

//  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  std::ostringstream converter;
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->CBData->lock);

    Common_UI_GTKBuildersIterator_t iterator =
        data_p->CBData->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
    // sanity check(s)
    ACE_ASSERT (iterator != data_p->CBData->builders.end ());

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
  } // end lock scope

  if (!data_p->CBData->stream->initialize (data_p->CBData->configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_CamSave_Stream::initialize(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  session_data_container_p = data_p->CBData->stream->get ();
  ACE_ASSERT (session_data_container_p);
  session_data_p = &session_data_container_p->get ();
  data_p->sessionID = session_data_p->sessionID;
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << session_data_p->sessionID;

  // generate context ID
  gdk_threads_enter ();
  data_p->CBData->configuration->streamConfiguration.moduleHandlerConfiguration_2.contextID =
    gtk_statusbar_get_context_id (statusbar_p,
                                  converter.str ().c_str ());
  gdk_threads_leave ();

  data_p->CBData->stream->start ();
  //if (!data_p->CBData->stream->isRunning ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_CamSave_Stream::start(): \"%m\", aborting\n")));
  //  goto done;
  //} // end IF
  data_p->CBData->stream->waitForCompletion (true, false);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

error:
  { // synch access
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->CBData->lock);
    data_p->CBData->progressData.completedActions.insert (data_p->eventSourceID);
  } // end lock scope

  // clean up
  delete data_p;

  return result;
}

/////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  Stream_CamSave_GTK_CBData* data_p = static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
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
  //  GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
  //                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
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
//    //GTK_WIDGET (glade_xml_get_widget ((*iterator).second.second,
//    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
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
  if (!load_capture_devices (list_store_p))
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
  if (!data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName.empty ())
  {
    GError* error_p = NULL;
    GFile* file_p =
      g_file_new_for_path (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName.c_str ());
    ACE_ASSERT (file_p);

    // *NOTE*: gtk does not complain if the file doesn't exist, but the button
    //         will display "(None)" --> create empty file
    if (!Common_File_Tools::isReadable (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName))
      if (!Common_File_Tools::create (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::create(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName.c_str ())));

        // clean up
        g_object_unref (file_p);

        return G_SOURCE_REMOVE;
      } // end IF

    //std::string file_uri =
    //  ACE_TEXT_ALWAYS_CHAR ("file://") +
    //  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName;
    //if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
    //                                              file_uri.c_str ()))
    if (!gtk_file_chooser_set_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                   file_p,
                                                   &error_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_file(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName.c_str ()),
                  ACE_TEXT (error_p->message)));

      // clean up
      g_error_free (error_p);
      g_object_unref (file_p);

      return G_SOURCE_REMOVE;
    } // end IF

    GtkFileChooserDialog* file_chooser_dialog_p =
      GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
    ACE_ASSERT (file_chooser_dialog_p);
    if (!gtk_file_chooser_select_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
                                       file_p,
                                       &error_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_select_file(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName.c_str ()),
                  ACE_TEXT (error_p->message)));

      // clean up
      g_error_free (error_p);
      g_object_unref (file_p);

      return G_SOURCE_REMOVE;
    } // end IF
    g_object_unref (file_p);
  } // end IF

  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

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
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
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
  gtk_rc_style_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  GtkDrawingArea* drawing_area_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  // step5: initialize updates
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

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
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
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

  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  result_2 =
    g_signal_connect (file_chooser_button_p,
                      ACE_TEXT_ALWAYS_CHAR ("file-set"),
                      G_CALLBACK (filechooserbutton_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  GtkFileChooserDialog* file_chooser_dialog_p =
    GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  ACE_ASSERT (file_chooser_dialog_p);
  result_2 =
    g_signal_connect (file_chooser_dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("file-activated"),
                      G_CALLBACK (filechooserdialog_cb),
                      NULL);
  ACE_ASSERT (result_2);

  // step6b: connect custom signals
  GObject* object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (toggle_action_record_activate_cb),
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
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RATE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_rate_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  result_2 =
    g_signal_connect (G_OBJECT (drawing_area_p),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (drawingarea_configure_event_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //-------------------------------------

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
  default_folder_uri +=
    data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName;
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

  // step10: retrieve window handle (and canvas coordinates)
  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.window =
    gdk_win32_window_get_impl_hwnd (window_p);
#else
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.window =
    window_p;
#endif
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (allocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.bottom =
    allocation.height;
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.left =
    allocation.x;
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.right =
    allocation.width;
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.top =
    allocation.y;
#else
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area =
    allocation;
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

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // clean up
  int result = -1;
  if (data_p->device != -1)
  {
    result = v4l2_close (data_p->device);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  data_p->device));
    data_p->device = -1;
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
idle_update_log_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
      //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
      //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
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
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    // sanity check
    if (data_p->logStack.empty ()) return TRUE; // G_SOURCE_CONTINUE

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
        return FALSE; // G_SOURCE_REMOVE
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

  Stream_CamSave_GTK_CBData* data_p =
      static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  { // synch access
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    if (data_p->eventStack.empty ()) return TRUE; // G_SOURCE_CONTINUE

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
            //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
            //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
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
            //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
            //                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
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
            //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
            //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        case STREAM_GTKEVENT_STATISTIC:
        {
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

  Stream_CamSave_GTK_ProgressData* data_p =
      static_cast<Stream_CamSave_GTK_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  // synch access
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->GTKState->lock);

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
  Stream_CamSave_PendingActionsIterator_t iterator_2;
  for (Stream_CamSave_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
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
    else if (exit_status)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("thread %d has joined (status was: %d)...\n"),
                  thread_id,
                  exit_status));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("thread %u has joined (status was: %@)...\n"),
                  thread_id,
                  exit_status));
#endif
    } // end IF

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

  // --> reschedule
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}
gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  // synch access
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), true);

  return G_SOURCE_REMOVE;
}

/////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void
toggle_action_record_activate_cb (GtkToggleAction* toggleAction_in,
                                  gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_start_activate_cb"));

  Stream_CamSave_GTK_CBData* data_p =
      static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->stream);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  // toggle record/stop ?
  const Stream_StateMachine_ControlState& status_r =
    data_p->stream->status ();
  if ((status_r == STREAM_STATE_RUNNING) ||
      (status_r == STREAM_STATE_PAUSED))
  {
    data_p->stream->stop (false, true);

    gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_RECORD);
    gtk_action_set_sensitive (GTK_ACTION (toggleAction_in), false);
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

    // stop progress reporting
    ACE_ASSERT (data_p->progressEventSourceID);
    {
      ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

      if (!g_source_remove (data_p->progressEventSourceID))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    data_p->progressEventSourceID));
      data_p->eventSourceIds.erase (data_p->progressEventSourceID);
      data_p->progressEventSourceID = 0;
    } // end lock scope
    GtkProgressBar* progressbar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    // *NOTE*: this disables "activity mode" (in Gtk2)
    gtk_progress_bar_set_fraction (progressbar_p, 0.0);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), false);

    return;
  } // end IF

  Stream_CamSave_ThreadData* thread_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;

  GtkSpinButton* spin_button_p = NULL;
  gdouble value_d = 0.0;

  if (data_p->isFirst)
    data_p->isFirst = false;

  // step0: modify widgets
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_STOP);
  GtkAction* action_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_REPORT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);
  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

  // step1: set up progress reporting
  data_p->progressData.statistic = Stream_Statistic ();
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
    data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.device =
      g_value_get_string (&value);
    g_value_unset (&value);
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.fileDescriptor =
      data_p->device;
#endif

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GFile* file_p =
    gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_button_p));
  if (file_p)
  {
    char* string_p = g_file_get_path (file_p);
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));

      // clean up
      g_object_unref (file_p);

      goto error;
    } // end IF
    g_object_unref (file_p);
    data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName =
      string_p;
    g_free (string_p);
  } // end IF
  spin_button_p =
    //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  value_d = gtk_spin_button_get_value (spin_button_p);
  if (value_d)
    data_p->configuration->streamConfiguration.bufferSize =
        static_cast<unsigned int> (value_d);
  else
    gtk_spin_button_set_value (spin_button_p,
                               static_cast<gdouble> (data_p->configuration->streamConfiguration.bufferSize));

  // step3: start processing thread
  ACE_NEW_NORETURN (thread_data_p,
                    Stream_CamSave_ThreadData ());
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  thread_data_p->CBData = data_p;
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
                  ACE_TEXT (TEST_U_STREAM_CAMSAVE_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    int result =
      thread_manager_p->spawn (::stream_processing_function,    // function
                               thread_data_p,                   // argument
                               (THR_NEW_LWP |
                                THR_JOINABLE |
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
    ACE_ASSERT (!data_p->progressEventSourceID);
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
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    thread_id));

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
} // toggle_action_record_activate_cb
void
action_cut_activate_cb (GtkAction* action_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_cut_activate_cb"));

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->stream);

  data_p->stream->control (STREAM_CONTROL_STEP,
                           false);
} // action_cut_activate_cb
void
action_report_activate_cb (GtkAction* action_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_report_activate_cb"));

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->stream);
} // action_report_activate_cb

// -----------------------------------------------------------------------------

gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
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
  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    //GTK_DIALOG (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR(TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
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
  Stream_CamSave_GTK_CBData* data_p = static_cast<Stream_CamSave_GTK_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->stream);

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

  // stop stream ?
  const Stream_StateMachine_ControlState& status_r =
    data_p->stream->status ();
  if ((status_r == STREAM_STATE_RUNNING) ||
      (status_r == STREAM_STATE_PAUSED))
    data_p->stream->stop (false, true);

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

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  GValue value_2 = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            0, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  std::string device_string = g_value_get_string (&value);
  std::string device_path = g_value_get_string (&value_2);
  g_value_unset (&value);
  g_value_unset (&value_2);

  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_UNUSED_ARG (device_path);

  if (data_p->streamConfiguration)
  {
    data_p->streamConfiguration->Release ();
    data_p->streamConfiguration = NULL;
  } // end IF
  if (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder)
  {
    data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder->Release ();
    data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder =
      NULL;
  } // end IF
  if (!Stream_Module_Device_Tools::load (device_string,
                                         data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder,
                                         data_p->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                ACE_TEXT (device_string.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder);
  ACE_ASSERT (data_p->streamConfiguration);
  if (!load_formats (data_p->streamConfiguration,
#else
  int result = -1;
  if (data_p->device != -1)
  {
    result = v4l2_close (data_p->device);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  data_p->device));
    data_p->device = -1;
  } // end IF
  ACE_ASSERT (data_p->device == -1);
  int open_mode =
      ((data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.method == V4L2_MEMORY_MMAP) ? O_RDWR
                                                                                                            : O_RDONLY);
  data_p->device = v4l2_open (device_path.c_str (),
                              open_mode);
  if (data_p->device == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", returning\n"),
                ACE_TEXT (device_path.c_str ()), open_mode));
    return;
  } // end IF

  if (!load_formats (data_p->device,
#endif
                     list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_formats(), returning\n")));
    return;
  } // end IF
  gint n_rows =
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

  GtkToggleAction* toggle_action_p =
      GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), true);
} // combobox_source_changed_cb

void
combobox_format_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

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
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  GUID GUID_i;
  ACE_OS::memset (&GUID_i, 0, sizeof (GUID));
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result =
    CLSIDFromString (format_string.c_str (),
                     &GUID_i);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (format_string.c_str ()),
                     &GUID_i);
#endif
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
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

  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (data_p->streamConfiguration);
  ACE_ASSERT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder);

  AM_MEDIA_TYPE* media_type_p = NULL;
  result = data_p->streamConfiguration->GetFormat (&media_type_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_type_p);
  media_type_p->subtype = GUID_i;

  // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
  //         the filter pins are associated. IGraphConfig::Reconnect fails
  //         unless the graph is "disconnected" first
  if (!Stream_Module_Device_Tools::disconnect (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), returning\n")));
    goto error;
  } // end IF
  if (!Stream_Module_Device_Tools::setFormat (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder,
                                              *media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), returning\n")));
    goto error;
  } // end IF
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  goto continue_;

error:
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  return;
#else
  // sanity check(s)
  ACE_ASSERT (data_p->device != -1);

  if (!Stream_Module_Device_Tools::setFormat (data_p->device,
                                              format_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), returning\n")));
    return;
  } // end IF

  goto continue_;
#endif
continue_:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_resolutions (data_p->streamConfiguration,
                         GUID_i,
#else
  if (!load_resolutions (data_p->device,
                         format_i,
#endif
                         list_store_p))
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

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

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
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  GUID GUID_i;
  ACE_OS::memset (&GUID_i, 0, sizeof (GUID));
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result =
    CLSIDFromString (g_value_get_string (&value),
                     &GUID_i);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (g_value_get_string (&value)),
                     &GUID_i);
#endif
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    g_value_unset (&value);

    return;
  } // end IF
#else
  __u32 format_i = 0;
  std::istringstream converter;
  converter.str (g_value_get_string (&value));
  converter >> format_i;
#endif
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
  GValue value_2 = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
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

  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (data_p->streamConfiguration);
  ACE_ASSERT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder);

  AM_MEDIA_TYPE* media_type_p = NULL;
  result = data_p->streamConfiguration->GetFormat (&media_type_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_type_p);
  if (media_type_p->formattype == FORMAT_VideoInfo)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
    video_info_header_p->bmiHeader.biWidth = width;
    video_info_header_p->bmiHeader.biHeight = height;
  } // end IF
  else if (media_type_p->formattype == FORMAT_VideoInfo2)
  {
    // *NOTE*: these media subtypes do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
    video_info_header2_p->bmiHeader.biWidth = width;
    video_info_header2_p->bmiHeader.biHeight = height;
  } // end ELSE IF

  // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
  //         the filter pins are associated. IGraphConfig::Reconnect fails
  //         unless the graph is "disconnected" first
  if (!Stream_Module_Device_Tools::disconnect (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), returning\n")));
    goto error;
  } // end IF
  if (!Stream_Module_Device_Tools::setFormat (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder,
                                              *media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), returning\n")));
    goto error;
  } // end IF
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  goto continue_;

error:
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  return;
#else
  // sanity check(s)
  ACE_ASSERT (data_p->device != -1);

  if (!Stream_Module_Device_Tools::setResolution (data_p->device,
                                                  width, height))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setResolution(), returning\n")));
    return;
  } // end IF

  goto continue_;
#endif
continue_:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_rates (data_p->streamConfiguration,
                   GUID_i,
                   width,
#else
  if (!load_rates (data_p->device,
                   format_i,
                   width, height,
#endif
                   list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_rates(), returning\n")));
    return;
  } // end IF

  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
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

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RATE_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  GValue value_2 = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_UINT);
  unsigned int frame_interval = g_value_get_uint (&value);
  g_value_unset (&value);
  unsigned int frame_interval_denominator = g_value_get_uint (&value_2);
  g_value_unset (&value_2);

  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_UNUSED_ARG (frame_interval_denominator);

  // sanity check(s)
  ACE_ASSERT (data_p->streamConfiguration);
  ACE_ASSERT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder);

  AM_MEDIA_TYPE* media_type_p = NULL;
  HRESULT result = data_p->streamConfiguration->GetFormat (&media_type_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_type_p);
  if (media_type_p->formattype == FORMAT_VideoInfo)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
    video_info_header_p->AvgTimePerFrame = frame_interval;
  } // end IF
  else if (media_type_p->formattype == FORMAT_VideoInfo2)
  {
    // *NOTE*: these media subtypes do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
    video_info_header2_p->AvgTimePerFrame = frame_interval;
  } // end ELSE IF

  // *NOTE*: the graph may (!) be stopped, but is in a "connected" state, i.e.
  //         the filter pins are associated. IGraphConfig::Reconnect fails
  //         unless the graph is "disconnected" first
  if (!Stream_Module_Device_Tools::disconnect (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), returning\n")));
    goto error;
  } // end IF
  if (!Stream_Module_Device_Tools::setFormat (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.builder,
                                              *media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(), returning\n")));
    goto error;
  } // end IF
    //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  return;

error:
  //DeleteMediaType (media_type_p);
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);
#else
  // sanity check(s)
  ACE_ASSERT (data_p->device != -1);

  struct v4l2_fract frame_interval_fract;
  ACE_OS::memset (&frame_interval_fract, 0, sizeof (struct v4l2_fract));
  frame_interval_fract.numerator = frame_interval;
  frame_interval_fract.denominator = frame_interval_denominator;
  if (!Stream_Module_Device_Tools::setInterval (data_p->device,
                                                frame_interval_fract))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setInterval(), returning\n")));
    return;
  } // end IF
#endif
} // combobox_rate_changed_cb

void
drawingarea_configure_event_cb (GtkWindow* window_in,
                                GdkEvent* event_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.window          ||
      !data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.windowController) // <-- window not realized yet ?
    return;
#else
  if (!data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.window) // <-- window not realized yet ?
    return;
#endif

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkDrawingArea* drawing_area_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.windowController);

  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.bottom =
    allocation.height;
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.left =
    allocation.x;
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.right =
    allocation.width;
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.top =
    allocation.y;

  //HRESULT result =
  //  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.windowController->SetWindowPosition (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.left,
  //                                                                                                               data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.top,
  //                                                                                                               data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.right,
  //                                                                                                               data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.bottom);
  //if (FAILED (result))
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
  //              data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.left, data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.top,
  //              data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.right, data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area.bottom,
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#else
  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.area =
    allocation;
#endif
} // drawingarea_configure_event_cb

void
filechooserbutton_cb (GtkFileChooserButton* button_in,
                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_cb"));

  Stream_CamSave_GTK_CBData* data_p =
    static_cast<Stream_CamSave_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  //ACE_ASSERT (iterator != data_p->gladeXML.end ());
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
  //gtk_entry_set_text (entry_p, string_p);

  data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName =
    Common_UI_Tools::UTF82Locale (string_p, -1);
  if (data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_Tools::UTF82Locale(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (string_p)));

    // clean up
    g_free (string_p);

    return;
  } // end IF
  g_free (string_p);

  // record button
  GtkToggleAction* toggle_action_p =
    //GTK_BUTTON (glade_xml_get_widget ((*iterator).second.second,
    //                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLOSE_NAME)));
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_sensitive (GTK_ACTION (toggle_action_p),
                            !data_p->configuration->streamConfiguration.moduleHandlerConfiguration_2.targetFileName.empty ());
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
