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
#include "stream_dev_tools.h"

#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <amvideo.h>
#include <d3d9.h>
#include <d3d9types.h>
#include <dvdmedia.h>
#include <dxva2api.h>
//#include <minwindef.h>
#include <strmif.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#include <windef.h>
#include <WinUser.h>
#endif // ACE_WIN32 || ACE_WIN64

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Dirent_Selector.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"

#include "common_error_tools.h"

#include "common_time_common.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
int
v4l_device_dirent_selector_cb (const dirent* dirEntry_in)
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
v4l_device_dirent_comparator_cb (const dirent** d1,
                                 const dirent** d2)
{
  return ACE_OS::strcmp ((*d1)->d_name,
                         (*d2)->d_name);
}
#endif // ACE_WIN32 || ACE_WIN64

void
Stream_Device_Tools::initialize (bool initializeFrameworks_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::initialize"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (initializeFrameworks_in)
  {
    Stream_Device_DirectShow_Tools::initialize (true); // initialize COM ?
    Stream_Device_MediaFoundation_Tools::initialize ();
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
void
Stream_Device_Tools::dump (struct _snd_pcm* deviceHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::dump"));

  struct _snd_pcm_hw_params* format_p = NULL;
  int result = -1;
  unsigned int rate_min, rate_max;
  int subunit_direction = 0;
  unsigned int period_time_min, period_time_max;
  snd_pcm_uframes_t period_size_min, period_size_max;
  unsigned int periods_min, periods_max;
  unsigned int buffer_time_min, buffer_time_max;
  snd_pcm_uframes_t buffer_size_min, buffer_size_max;

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_any (deviceHandle_in, format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_rate_min (format_p,
                                           &rate_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_rate_max (format_p,
                                           &rate_max, &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: available rates: %u-%u...\n"),
              ACE_TEXT (snd_pcm_name (deviceHandle_in)),
              rate_min, rate_max));

  result =
    snd_pcm_hw_params_get_period_time_min (format_p,
                                           &period_time_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result =
    snd_pcm_hw_params_get_period_time_max (format_p,
                                           &period_time_max, &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: available period times: %u-%u (us)...\n"),
              ACE_TEXT (snd_pcm_name (deviceHandle_in)),
              period_time_min, period_time_max));
  result =
    snd_pcm_hw_params_get_period_size_min (format_p,
                                           &period_size_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result =
    snd_pcm_hw_params_get_period_size_max (format_p,
                                           &period_size_max, &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: available period sizes: %u-%u (frames)...\n"),
              ACE_TEXT (snd_pcm_name (deviceHandle_in)),
              period_size_min, period_size_max));
  result = snd_pcm_hw_params_get_periods_min (format_p,
                                              &periods_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_periods_max (format_p,
                                              &periods_max, &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: available periods: %u-%u (frames)...\n"),
              ACE_TEXT (snd_pcm_name (deviceHandle_in)),
              periods_min, periods_max));

  result =
    snd_pcm_hw_params_get_buffer_time_min (format_p,
                                           &buffer_time_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result =
    snd_pcm_hw_params_get_buffer_time_max (format_p,
                                           &buffer_time_max, &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: available buffer times: %u-%u (us)...\n"),
              ACE_TEXT (snd_pcm_name (deviceHandle_in)),
              buffer_time_min, buffer_time_max));
  result = snd_pcm_hw_params_get_buffer_size_min (format_p,
                                                  &buffer_size_min);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_buffer_size_max (format_p,
                                                  &buffer_size_max);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: available buffer sizes: %u-%u (frames)...\n"),
              ACE_TEXT (snd_pcm_name (deviceHandle_in)),
              buffer_size_min, buffer_size_max));

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);
}

bool
Stream_Device_Tools::canOverlay (int fileDescriptor_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::canOverlay"));

  int result = -1;

  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return false;
  } // end IF

  return (device_capabilities.device_caps & V4L2_CAP_VIDEO_OVERLAY);
}
bool
Stream_Device_Tools::canStream (int fileDescriptor_in)//,
//                                enum v4l2_memory method_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::canStream"));

  int result = -1;

  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return false;
  } // end IF

//  struct v4l2_requestbuffers request_buffers;
//  ACE_OS::memset (&request_buffers, 0, sizeof (struct v4l2_requestbuffers));
//  request_buffers.count = 0;
//  request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//  request_buffers.memory = method_in;
//  result = v4l2_ioctl (fileDescriptor_in,
//                       VIDIOC_REQBUFS,
//                       &request_buffers);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
//                fileDescriptor_in, ACE_TEXT ("VIDIOC_REQBUFS")));
//    return false;
//  } // end IF

  return ((device_capabilities.device_caps & V4L2_CAP_VIDEO_CAPTURE) &&
          (device_capabilities.device_caps & V4L2_CAP_STREAMING));// &&
//          (request_buffers.capabilities & V4L2_BUF_CAP_SUPPORTS_USERPTR));
}

Stream_Device_List_t
Stream_Device_Tools::getVideoCaptureDevices ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getVideoCaptureDevices"));

  // initialize return value(s)
  Stream_Device_List_t return_value;

  std::string directory_string (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY));
  ACE_Dirent_Selector entries;
  int result = entries.open (ACE_TEXT (directory_string.c_str ()),
                             &v4l_device_dirent_selector_cb,
                             &v4l_device_dirent_comparator_cb);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Dirent_Selector::open(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (directory_string.c_str ())));
    return return_value;
  } // end IF
#if defined (_DEBUG)
  if (!entries.length ())
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no video capture devices found, continuing\n")));
#endif // _DEBUG

  struct Stream_Device_Identifier device_identifier_s;
  struct v4l2_capability device_capabilities;
  std::string device_file_path;
  ACE_DIRENT* dirent_p = NULL;
  int file_descriptor = -1;
  int open_mode = O_RDONLY;
  for (unsigned int i = 0;
       i < static_cast<unsigned int> (entries.length ());
       ++i)
  {
    dirent_p = entries[i];
    ACE_ASSERT (dirent_p);
    device_file_path = directory_string;
    device_file_path += ACE_DIRECTORY_SEPARATOR_CHAR;
    device_file_path += dirent_p->d_name;
    ACE_ASSERT (Common_File_Tools::isValidFilename (device_file_path));
//    ACE_ASSERT (Common_File_Tools::isReadable (device_file_path));

    file_descriptor = v4l2_open (device_file_path.c_str (),
                                 open_mode);
    if (unlikely (file_descriptor == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", continuing\n"),
                  ACE_TEXT (device_file_path.c_str ()), open_mode));
      goto close;
    } // end IF

    ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
    result = v4l2_ioctl (file_descriptor,
                         VIDIOC_QUERYCAP,
                         &device_capabilities);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  file_descriptor, ACE_TEXT ("VIDIOC_QUERYCAP")));
      goto close;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("found video capture device \"%s\" (driver: \"%s\") on bus \"%s\"\n"),
                ACE_TEXT (reinterpret_cast<char*> (device_capabilities.card)),
                ACE_TEXT (reinterpret_cast<char*> (device_capabilities.driver)),
                ACE_TEXT (reinterpret_cast<char*> (device_capabilities.bus_info))));
#endif // _DEBUG

    device_identifier_s.description =
        ACE_TEXT_ALWAYS_CHAR (reinterpret_cast<char*> (device_capabilities.card));
    device_identifier_s.identifier = device_file_path;
    return_value.push_back (device_identifier_s);

close:
    result = v4l2_close (file_descriptor);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  file_descriptor));
    file_descriptor = -1;
  } // end FOR

  result = entries.close ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Dirent_Selector::close(\"%s\"): \"%m\", continuing\n"),
                ACE_TEXT (directory_string.c_str ())));

  return return_value;
}

struct v4l2_pix_format
Stream_Device_Tools::getVideoCaptureFormat (int fileDescriptor_in,
                                            __u32 pixelFormat_in,
                                            const Common_UI_Resolution_t& resolution_in,
                                            const struct v4l2_fract& frameRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getVideoCaptureFormat"));

  // initialize return value(s)
  struct v4l2_pix_format return_value;
  ACE_OS::memset (&return_value, 0, sizeof (struct v4l2_pix_format));
//  return_value.width = width_in;
//  return_value.height = height_in;
//  return_value.pixelformat = pixelFormat_in;
//  return_value.field = ;
//  return_value.bytesperline = ;
//  return_value.sizeimage = ;
//  return_value.colorspace = ;
//  return_value.priv = ;
//  return_value.flags = ;
//  return_value.ycbcr_enc = ;
//  return_value.quantization = ;
//  return_value.xfer_func = ;

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  int result = -1;
  struct v4l2_frmivalenum frmivalenum_s;
  for (__u32 i = 0;
       ;
       ++i)
  {
    ACE_OS::memset (&frmivalenum_s, 0, sizeof (struct v4l2_frmivalenum));
    frmivalenum_s.index = i;
    frmivalenum_s.pixel_format = pixelFormat_in;
    frmivalenum_s.width = resolution_in.width;
    frmivalenum_s.height = resolution_in.height;
    result = v4l2_ioctl (fileDescriptor_in,
                         VIDIOC_ENUM_FRAMEINTERVALS,
                         &frmivalenum_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FRAMEINTERVALS")));
    } // end IF
    ACE_ASSERT (frmivalenum_s.pixel_format == pixelFormat_in);
    ACE_ASSERT (frmivalenum_s.width == resolution_in.width);
    ACE_ASSERT (frmivalenum_s.height == resolution_in.height);
    ACE_ASSERT (frmivalenum_s.type == V4L2_FRMIVAL_TYPE_DISCRETE);
    if (((frameRate_in.denominator == frmivalenum_s.discrete.numerator) &&
        (frameRate_in.numerator == frmivalenum_s.discrete.denominator)) ||
        ((frameRate_in.denominator == 1) && (frameRate_in.numerator == 0))) // <-- don't care
    {
      return_value.width = resolution_in.width;
      return_value.height = resolution_in.height;
      return_value.pixelformat = pixelFormat_in;
      break;
    } // end IF
  } // end FOR
  if (!return_value.pixelformat)
    return return_value;

  // (try to) retrieve the missing format information
  struct v4l2_format format_s;
  ACE_OS::memset (&format_s, 0, sizeof (struct v4l2_format));
  format_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format_s.fmt.pix.pixelformat = return_value.pixelformat;
  format_s.fmt.pix.width = return_value.width;
  format_s.fmt.pix.height = return_value.height;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_TRY_FMT,
                       &format_s);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_TRY_FMT")));
    return return_value;
  } // end IF
  return_value = format_s.fmt.pix;

  return return_value;
}

Stream_MediaFramework_V4L_CaptureFormats_t
Stream_Device_Tools::getCaptureSubFormats (int fileDescriptor_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getCaptureSubFormats"));

  // initialize return value(s)
  Stream_MediaFramework_V4L_CaptureFormats_t return_value;

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  int result = -1;
  struct v4l2_fmtdesc fmtdesc_s;
  for (__u32 i = 0;
       ;
       ++i)
  {
    ACE_OS::memset (&fmtdesc_s, 0, sizeof (struct v4l2_fmtdesc));
    fmtdesc_s.index = i;
    fmtdesc_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    result = v4l2_ioctl (fileDescriptor_in,
                         VIDIOC_ENUM_FMT,
                         &fmtdesc_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FMT")));
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("video capture device (fd: %d) has format \"%s\"\n"),
                fileDescriptor_in,
                ACE_TEXT (reinterpret_cast<char*> (fmtdesc_s.description))));
#endif // _DEBUG

    return_value.push_back (std::make_pair (fmtdesc_s.pixelformat,
                                            reinterpret_cast<char*> (fmtdesc_s.description)));
  } // end FOR

  return return_value;
}

Common_UI_Resolutions_t
Stream_Device_Tools::getCaptureResolutions (int fileDescriptor_in,
                                            __u32 pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getCaptureResolutions"));

  // initialize return value(s)
  Common_UI_Resolutions_t return_value;

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  int result = -1;
  struct v4l2_frmsizeenum frmsizeenum_s;
  Common_UI_Resolution_t resolution_s;
  for (__u32 i = 0;
       ;
       ++i)
  {
    ACE_OS::memset (&frmsizeenum_s, 0, sizeof (struct v4l2_frmsizeenum));
    frmsizeenum_s.index = i;
    frmsizeenum_s.pixel_format = pixelFormat_in;
    result = v4l2_ioctl (fileDescriptor_in,
                         VIDIOC_ENUM_FRAMESIZES,
                         &frmsizeenum_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FRAMESIZES")));
    } // end IF
    ACE_ASSERT (frmsizeenum_s.pixel_format == pixelFormat_in);
    ACE_ASSERT (frmsizeenum_s.type == V4L2_FRMSIZE_TYPE_DISCRETE);
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("video capture device (fd: %d, format: \"%s\") has resolution %dx%d\n"),
                fileDescriptor_in,
                ACE_TEXT (Stream_Device_Tools::formatToString (fileDescriptor_in, pixelFormat_in).c_str ()),
                frmsizeenum_s.discrete.width, frmsizeenum_s.discrete.height));
#endif // _DEBUG
    resolution_s.width = frmsizeenum_s.discrete.width;
    resolution_s.height = frmsizeenum_s.discrete.height;
    return_value.push_back (resolution_s);
  } // end FOR

  return return_value;
}

Common_UI_Framerates_t
Stream_Device_Tools::getCaptureFramerates (int fileDescriptor_in,
                                           __u32 pixelFormat_in,
                                           const Common_UI_Resolution_t& resolution_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getCaptureResolutions"));

  // initialize return value(s)
  Common_UI_Framerates_t return_value;

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);
  ACE_ASSERT (pixelFormat_in);

  int result = -1;
  struct v4l2_frmivalenum frmivalenum_s;
  for (__u32 i = 0;
       ;
       ++i)
  {
    ACE_OS::memset (&frmivalenum_s, 0, sizeof (struct v4l2_frmivalenum));
    frmivalenum_s.index = i;
    frmivalenum_s.pixel_format = pixelFormat_in;
    frmivalenum_s.width = resolution_in.width;
    frmivalenum_s.height = resolution_in.height;
    result = v4l2_ioctl (fileDescriptor_in,
                         VIDIOC_ENUM_FRAMEINTERVALS,
                         &frmivalenum_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FRAMEINTERVALS")));
    } // end IF
    ACE_ASSERT (frmivalenum_s.pixel_format == pixelFormat_in);
    ACE_ASSERT (frmivalenum_s.width == resolution_in.width);
    ACE_ASSERT (frmivalenum_s.height == resolution_in.height);
    ACE_ASSERT (frmivalenum_s.type == V4L2_FRMIVAL_TYPE_DISCRETE);
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("video capture device (fd: %d, format: \"%s\", resolution: %dx%d) has framerate %d/%d\n"),
                fileDescriptor_in,
                ACE_TEXT (Stream_Device_Tools::formatToString (fileDescriptor_in, pixelFormat_in).c_str ()),
                resolution_in.width, resolution_in.height,
                frmivalenum_s.discrete.denominator, frmivalenum_s.discrete.numerator));
#endif // _DEBUG

    return_value.push_back (frmivalenum_s.discrete.denominator);
  } // end FOR

  return return_value;
}

struct Stream_MediaFramework_V4L_MediaType
Stream_Device_Tools::defaultCaptureFormat (const std::string& deviceIdentifier_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::defaultCaptureFormat"));

  // initialize return value(s)
  struct Stream_MediaFramework_V4L_MediaType return_value;

  // sanity check(s)
  ACE_ASSERT (!deviceIdentifier_in.empty ());

  // *NOTE*: use O_NONBLOCK with a reactor (v4l2_select()) or proactor
  //         (v4l2_poll()) for asynchronous operation
  // *TODO*: support O_NONBLOCK
  int open_mode = O_RDONLY;
  int result = -1;
  int file_descriptor = v4l2_open (deviceIdentifier_in.c_str (),
                                   open_mode);
  if (unlikely (file_descriptor == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", aborting\n"),
                ACE_TEXT (deviceIdentifier_in.c_str ()),
                open_mode));
    return return_value;
  } // end IF

  struct v4l2_format format_s;
  ACE_OS::memset (&format_s, 0, sizeof (struct v4l2_format));
  if (unlikely (!Stream_Device_Tools::getFormat (file_descriptor,
                                                 format_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::getFormat(%d), aborting\n"),
                file_descriptor));
    goto close;
  } // end IF
  ACE_ASSERT (format_s.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);
  return_value.format = format_s.fmt.pix;
  if (unlikely (!Stream_Device_Tools::getFrameRate (file_descriptor,
                                                    return_value.frameRate)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::getFrameRate(%d), aborting\n"),
                file_descriptor));
    ACE_OS::memset (&return_value, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
    goto close;
  } // end IF

close:
  result = v4l2_close (file_descriptor);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                file_descriptor));

  return return_value;
}

void
Stream_Device_Tools::dump (int fileDescriptor_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::dump"));

  int result = -1;

  // sanity check(s)
  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", returning\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return;
  } // end IF
  std::ostringstream converter;
  converter << ((device_capabilities.version >> 16) & 0xFF)
            << '.'
            << ((device_capabilities.version >> 8) & 0xFF)
            << '.'
            << (device_capabilities.version & 0xFF);
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("device file descriptor: %d\n---------------------------\ndriver: \"%s\"\ncard: \"%s\"\nbus: \"%s\"\nversion: \"%s\"\ncapabilities: %u\ndevice capabilities: %u\nreserved: %u|%u|%u\n"),
              fileDescriptor_in,
              ACE_TEXT (device_capabilities.driver),
              ACE_TEXT (device_capabilities.card),
              ACE_TEXT (device_capabilities.bus_info),
              ACE_TEXT (converter.str ().c_str ()),
              device_capabilities.capabilities,
              device_capabilities.device_caps,
              device_capabilities.reserved[0],device_capabilities.reserved[1],device_capabilities.reserved[2]));
}

std::string
Stream_Device_Tools::getDeviceName (enum _snd_pcm_stream direction_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getDeviceName"));

  std::string result_string;

  // sanity check(s)
  ACE_ASSERT ((direction_in == SND_PCM_STREAM_CAPTURE) ||
              (direction_in == SND_PCM_STREAM_PLAYBACK));

  void** hints_p = NULL;
  int result =
      snd_device_name_hint (-1,
                            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_ALSA_PCM_INTERFACE_NAME),
                            &hints_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_device_name_hint(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    return result_string;
  } // end IF

  char* string_p = NULL;
  std::string hint_string, device_type;
  std::string::size_type position = std::string::npos;
  for (void** i = hints_p; *i; ++i)
  {
    string_p = NULL;
    string_p = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("IOID"));
    if (!string_p)
    { // *NOTE*: NULL: device is i/o
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("failed to snd_device_name_get_hint(\"IOID\"): \"%m\", continuing\n")));
      goto continue_;
    } // end IF
    hint_string = string_p;
    free (string_p);
    string_p = NULL;
    if (ACE_OS::strcmp (hint_string.c_str (),
                        (direction_in == SND_PCM_STREAM_PLAYBACK) ? ACE_TEXT_ALWAYS_CHAR ("Output")
                                                                  : ACE_TEXT_ALWAYS_CHAR ("Input")))
      continue;

continue_:
    string_p = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("NAME"));
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    hint_string = string_p;
    free (string_p); string_p = NULL;

    // filter hardware devices
    device_type = hint_string;
    position = hint_string.find_first_of (':');
    if (position != std::string::npos)
      device_type = device_type.substr (0, position);
    if (ACE_OS::strcmp (device_type.c_str (),
                        (direction_in == SND_PCM_STREAM_PLAYBACK) ? ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_ALSA_DEVICE_PLAYBACK_PREFIX)
                                                                  : ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_ALSA_DEVICE_CAPTURE_PREFIX)))
      continue;
    result_string = hint_string;

//    string_p = snd_device_name_get_hint (*i, "DESC");
//    if (!string_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", aborting\n")));
//      goto clean;
//    } // end IF

//    // clean up
//    free (string_p); string_p = NULL;
    break;
  } // end FOR

clean:
  if (hints_p)
  {
    result = snd_device_name_free_hint (hints_p);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_free_hint(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF

  return result_string;
}

bool
Stream_Device_Tools::initializeCapture (int fileDescriptor_in,
                                        enum v4l2_memory method_in,
                                        __u32& numberOfBuffers_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::initializeCapture"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (Stream_Device_Tools::canStream (fileDescriptor_in));

  struct v4l2_requestbuffers request_buffers;
  ACE_OS::memset (&request_buffers, 0, sizeof (struct v4l2_requestbuffers));
  request_buffers.count = numberOfBuffers_inout;
  request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request_buffers.memory = method_in;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_REQBUFS,
                       &request_buffers);
  if (result == -1)
  { int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_REQBUFS")));
      return false;
    } // end IF
    goto no_support;
  } // end IF
  numberOfBuffers_inout = request_buffers.count;
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocated %d device (fd: %d) buffer slots\n"),
              numberOfBuffers_inout,
              fileDescriptor_in));
#endif // _DEBUG
  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("device (fd was: %d) does not support streaming method (was: %d), aborting\n"),
              fileDescriptor_in, method_in));

  return false;
}
bool
Stream_Device_Tools::initializeOverlay (int fileDescriptor_in,
                                        const struct v4l2_window& window_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::initializeOverlay"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (Stream_Device_Tools::canOverlay (fileDescriptor_in));

  // step1: set up frame-buffer (if necessary)
  struct v4l2_framebuffer framebuffer_s;
  ACE_OS::memset (&framebuffer_s, 0, sizeof (struct v4l2_framebuffer));
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_G_FBUF,
                       &framebuffer_s);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FBUF")));
      return false;
    } // end IF
    goto no_support;
  } //IF end

  // *TODO*: configure frame-buffer options

  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_S_FBUF,
                       &framebuffer_s);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_S_FBUF")));
    goto error;
  } // end IF

  // step2: set up output format / overlay window
  struct v4l2_format format;
  ACE_OS::memset (&format, 0, sizeof (struct v4l2_format));
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_G_FMT,
                       &format);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FMT")));
    goto error;
  } // end IF

  format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
  format.fmt.win = window_in;
  // *TODO*: configure format options

  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_S_FMT,
                       &format);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_S_FMT")));
    goto error;
  } // end IF
  // *TODO*: verify that format now contains the requested configuration

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("device (was: %d) does not support overlays, aborting\n"),
              fileDescriptor_in));
error:
  return false;
}

unsigned int
Stream_Device_Tools::queued (int fileDescriptor_in,
                             unsigned int numberOfBuffers_in,
                             unsigned int& done_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::queued"));

  unsigned int result = 0;

  // init return value(s)
  done_out = 0;

  int result_2 = -1;
  struct v4l2_buffer buffer;
  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for (unsigned int i = 0;
       i < numberOfBuffers_in;
       ++i)
  {
    buffer.index = i;
    result_2 = v4l2_ioctl (fileDescriptor_in,
                           VIDIOC_QUERYBUF,
                           &buffer);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_QUERYBUF")));

    if (buffer.flags & V4L2_BUF_FLAG_DONE)
      ++done_out;
    if (buffer.flags & V4L2_BUF_FLAG_QUEUED)
      ++result;
  } // end FOR

  return result;
}

bool
Stream_Device_Tools::setFormat (struct _snd_pcm* deviceHandle_in,
                                const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::setFormat"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);

  struct _snd_pcm_hw_params* snd_pcm_hw_params_p = NULL;
//  struct Stream_MediaFramework_ALSA_MediaType& media_type_s =
//      const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in);
  int subunit_direction = 0;

  snd_pcm_hw_params_malloc (&snd_pcm_hw_params_p);
  if (unlikely (!snd_pcm_hw_params_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = snd_pcm_hw_params_any (deviceHandle_in, snd_pcm_hw_params_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_access (deviceHandle_in, snd_pcm_hw_params_p,
                                         mediaType_in.access);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_format (deviceHandle_in, snd_pcm_hw_params_p,
                                         mediaType_in.format);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
      snd_pcm_hw_params_set_channels (deviceHandle_in, snd_pcm_hw_params_p,
                                      mediaType_in.channels);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_channels(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
      snd_pcm_hw_params_set_rate_resample (deviceHandle_in, snd_pcm_hw_params_p,
                                           0);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_rate_resample(0): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_rate_near (deviceHandle_in, snd_pcm_hw_params_p,
                                       &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).rate,
                                       &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_rate_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
      snd_pcm_hw_params_set_period_time_near (deviceHandle_in, snd_pcm_hw_params_p,
                                              &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).periodTime,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_period_time_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_period_size_near (deviceHandle_in, snd_pcm_hw_params_p,
                                              &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).periodSize,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_period_size_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_periods_near (deviceHandle_in, snd_pcm_hw_params_p,
                                          &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).periods,
                                          &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_periods_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
      snd_pcm_hw_params_set_buffer_time_near (deviceHandle_in, snd_pcm_hw_params_p,
                                              &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).bufferTime,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_buffer_time_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_buffer_size_near (deviceHandle_in, snd_pcm_hw_params_p,
                                              &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).bufferSize);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_buffer_size_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params (deviceHandle_in,
                              snd_pcm_hw_params_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  snd_pcm_hw_params_free (snd_pcm_hw_params_p); snd_pcm_hw_params_p = NULL;

  return true;

error:
  if (snd_pcm_hw_params_p)
    snd_pcm_hw_params_free (snd_pcm_hw_params_p);

  return false;
}
bool
Stream_Device_Tools::getFormat (struct _snd_pcm* deviceHandle_in,
                                struct Stream_MediaFramework_ALSA_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getFormat"));

  int result = -1;
//  bool free_format = false;

  // initialize return value(s)
  ACE_OS::memset (&mediaType_out,
                  0,
                  sizeof (struct Stream_MediaFramework_ALSA_MediaType));

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);

  struct _snd_pcm_hw_params* snd_pcm_hw_params_p = NULL;
  unsigned int sample_rate_numerator, sample_rate_denominator;
  int subunit_direction = 0;
//  unsigned int rate_resample;

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&snd_pcm_hw_params_p);
  if (unlikely (!snd_pcm_hw_params_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
//  result = snd_pcm_hw_params_any (deviceHandle_in, format_p);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
//                ACE_TEXT (snd_strerror (result))));
//    goto error;
//  } // end IF

  result = snd_pcm_hw_params_current (deviceHandle_in,
                                      snd_pcm_hw_params_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_current(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  ACE_ASSERT (snd_pcm_hw_params_p);

  result = snd_pcm_hw_params_get_access (snd_pcm_hw_params_p,
                                         &mediaType_out.access);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_format (snd_pcm_hw_params_p,
                                         &mediaType_out.format);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_subformat (snd_pcm_hw_params_p,
                                            &mediaType_out.subFormat);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_subformat(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_channels (snd_pcm_hw_params_p,
                                           &mediaType_out.channels);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_rate_numden (snd_pcm_hw_params_p,
                                              &sample_rate_numerator,
                                              &sample_rate_denominator);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_numden(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  mediaType_out.rate = sample_rate_numerator;
  ACE_ASSERT (sample_rate_denominator == 1);
//  result = snd_pcm_hw_params_get_rate_resample (deviceHandle_in, format_in,
//                                                  &rate_resample);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_resample(): \"%s\", aborting\n"),
//                ACE_TEXT (snd_strerror (result))));
//    goto error;
//  } // end IF
//  result += ACE_TEXT_ALWAYS_CHAR ("rate resample: ");
//  result += (rate_resample ? ACE_TEXT_ALWAYS_CHAR ("yes")
//                           : ACE_TEXT_ALWAYS_CHAR ("no"));
//  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result = snd_pcm_hw_params_get_period_time (snd_pcm_hw_params_p,
                                              &mediaType_out.periodTime,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_period_size (snd_pcm_hw_params_p,
                                              &mediaType_out.periodSize,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_periods (snd_pcm_hw_params_p,
                                          &mediaType_out.periods,
                                          &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_periods(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_buffer_time (snd_pcm_hw_params_p,
                                              &mediaType_out.bufferTime,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_buffer_size (snd_pcm_hw_params_p,
                                              &mediaType_out.bufferSize);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  snd_pcm_hw_params_free (snd_pcm_hw_params_p); snd_pcm_hw_params_p = NULL;

  return true;

error:
  if (snd_pcm_hw_params_p)
    snd_pcm_hw_params_free (snd_pcm_hw_params_p);

  return false;
}

bool
Stream_Device_Tools::setFormat (int fileDescriptor_in,
                                const struct v4l2_pix_format& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::setFormat"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  struct v4l2_format format_s;
  ACE_OS::memset (&format_s, 0, sizeof (struct v4l2_format));
  format_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_G_FMT,
                       &format_s);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF
  format_s.fmt.pix.pixelformat = format_in.pixelformat;
  format_s.fmt.pix.width = format_in.width;
  format_s.fmt.pix.height = format_in.height;

  format_s.fmt.pix.bytesperline = 0;
//  format_s.fmt.pix.priv = 0;
  format_s.fmt.pix.sizeimage = 0;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_S_FMT,
                       &format_s);
  if (result == -1)
  {// int error = ACE_OS::last_error (); ACE_UNUSED_ARG (error);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_S_FMT")));
    return false;
  } // end IF

  return true;
}
bool
Stream_Device_Tools::getFormat (int fileDescriptor_in,
                                struct v4l2_format& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getFormat"));

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  int result = -1;
  ACE_OS::memset (&format_out, 0, sizeof (struct v4l2_format));
  format_out.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_G_FMT,
                       &format_out);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF
//  ACE_ASSERT (format_out.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  return true;
}
bool
Stream_Device_Tools::getFrameRate (int fileDescriptor_in,
                                   struct v4l2_fract& frameRate_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getFrameRate"));

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  // initialize return value(s)
  ACE_OS::memset (&frameRate_out, 0, sizeof (struct v4l2_fract));

  int result = -1;
  struct v4l2_streamparm stream_parameters;
  ACE_OS::memset (&stream_parameters, 0, sizeof (struct v4l2_streamparm));
  stream_parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_G_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_PARM")));
    return false;
  } // end IF
  if ((stream_parameters.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) == 0)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the device driver does not support frame interval settings, continuing\n")));

  //  ACE_ASSERT (stream_parameters.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  // *NOTE*: the frame rate is the reciprocal value of the time-per-frame
  //         interval
  frameRate_out.denominator =
      stream_parameters.parm.capture.timeperframe.numerator;
  frameRate_out.numerator =
      stream_parameters.parm.capture.timeperframe.denominator;

  return true;
}
bool
Stream_Device_Tools::setFrameRate (int fileDescriptor_in,
                                   const struct v4l2_fract& frameRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::setFrameRate"));

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  int result = -1;
  struct v4l2_streamparm stream_parameters;
  ACE_OS::memset (&stream_parameters, 0, sizeof (struct v4l2_streamparm));
  stream_parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_G_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_PARM")));
    return false;
  } // end IF
//  ACE_ASSERT (stream_parameters.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);
  // sanity check(s)
  if ((stream_parameters.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) == 0)
    goto no_support;
  if ((stream_parameters.parm.capture.timeperframe.numerator   == frameRate_in.denominator)  &&
      (stream_parameters.parm.capture.timeperframe.denominator == frameRate_in.numerator))
    return true; // nothing to do

  // *NOTE*: v4l expects time-per-frame (s) --> pass reciprocal value
  stream_parameters.parm.capture.timeperframe.numerator =
      frameRate_in.denominator;
  stream_parameters.parm.capture.timeperframe.denominator =
      frameRate_in.numerator;
  result = v4l2_ioctl (fileDescriptor_in,
                       VIDIOC_S_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_S_PARM")));
    return false;
  } // end IF

  // validate setting
  if ((stream_parameters.parm.capture.timeperframe.numerator   != frameRate_in.denominator)  ||
      (stream_parameters.parm.capture.timeperframe.denominator != frameRate_in.numerator))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the device driver has not accepted the supplied frame rate (requested: %u/%u, is: %u/%u), continuing\n"),
                frameRate_in.numerator, frameRate_in.denominator,
                stream_parameters.parm.capture.timeperframe.denominator, stream_parameters.parm.capture.timeperframe.numerator));

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("the device driver does not support frame interval settings, aborting\n")));
  return false;
}

std::string
Stream_Device_Tools::formatToString (int fileDescriptor_in,
                                     uint32_t pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::formatToString"));

  std::string result;

  struct v4l2_fmtdesc fmtdesc_s;
  int result_2 = -1;
  for (__u32 i = 0;
       ;
       ++i)
  {
    ACE_OS::memset (&fmtdesc_s, 0, sizeof (struct v4l2_fmtdesc));
    fmtdesc_s.index = i;
    fmtdesc_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    result_2 = v4l2_ioctl (fileDescriptor_in,
                           VIDIOC_ENUM_FMT,
                           &fmtdesc_s);
    if (unlikely (result_2 == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FMT")));
      return result;
    } // end IF
    if (fmtdesc_s.pixelformat == pixelFormat_in)
    {
      result = reinterpret_cast<char*> (&fmtdesc_s.description);
      break;
    } // end IF
  } // end FOR

  return result;
}

std::string
Stream_Device_Tools::formatToString (const struct _snd_pcm_hw_params* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::formatToString"));

  std::string result;

  // sanity check(s)
  ACE_ASSERT (format_in);

  std::ostringstream converter;
  enum _snd_pcm_access access;
  enum _snd_pcm_format format;
  enum _snd_pcm_subformat sub_format;
  unsigned int channels;
  unsigned int sample_rate_numerator, sample_rate_denominator;
  int subunit_direction = 0;
  unsigned int period_time;
  snd_pcm_uframes_t period_size;
  unsigned int periods;
  unsigned int buffer_time;
  snd_pcm_uframes_t buffer_size;
//  unsigned int rate_resample;

  int result_2 = snd_pcm_hw_params_get_access (format_in,
                                               &access);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("access: ");
  result += snd_pcm_access_name (access);
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_format (format_in,
                                           &format);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("format: ");
  result += snd_pcm_format_name (format);
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_subformat (format_in,
                                              &sub_format);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_subformat(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("subformat: ");
  result += snd_pcm_subformat_name (sub_format);
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_channels (format_in,
                                             &channels);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("channels: ");
  converter << channels;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_rate_numden (format_in,
                                                &sample_rate_numerator,
                                                &sample_rate_denominator);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_numden(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("rate: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << sample_rate_numerator;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("/");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << sample_rate_denominator;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
//  result_2 = snd_pcm_hw_params_get_rate_resample (deviceHandle_in, format_in,
//                                                  &rate_resample);
//  if (result_2 < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_resample(): \"%s\", aborting\n"),
//                ACE_TEXT (snd_strerror (result_2))));
//    goto error;
//  } // end IF
//  result += ACE_TEXT_ALWAYS_CHAR ("rate resample: ");
//  result += (rate_resample ? ACE_TEXT_ALWAYS_CHAR ("yes")
//                           : ACE_TEXT_ALWAYS_CHAR ("no"));
//  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_period_time (format_in,
                                                &period_time,
                                                &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("period time: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << period_time;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result_2 = snd_pcm_hw_params_get_period_size (format_in,
                                                &period_size,
                                                &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("period size: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << period_size;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result_2 = snd_pcm_hw_params_get_periods (format_in,
                                            &periods,
                                            &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_periods(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("periods: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << periods;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_buffer_time (format_in,
                                                &buffer_time,
                                                &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("buffer time: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << buffer_time;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result_2 = snd_pcm_hw_params_get_buffer_size (format_in,
                                                &buffer_size);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("buffer size: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << buffer_size;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] supports sample-resolution mmap: ");
  result +=
      (snd_pcm_hw_params_can_mmap_sample_resolution (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                                : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] double buffering (start/stop): ");
  result +=
      (snd_pcm_hw_params_is_double (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                               : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] double buffering (data transfers): ");
  result +=
      (snd_pcm_hw_params_is_batch (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                              : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] sample block transfer: ");
  result +=
      (snd_pcm_hw_params_is_block_transfer (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                       : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] monotonic timestamps: ");
  result +=
      (snd_pcm_hw_params_is_monotonic (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                  : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports overrange detection: ");
  result +=
      (snd_pcm_hw_params_can_overrange (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                   : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports pause: ");
  result +=
      (snd_pcm_hw_params_can_pause (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                               : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports resume: ");
  result +=
      (snd_pcm_hw_params_can_resume (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] half-duplex only: ");
  result +=
      (snd_pcm_hw_params_is_half_duplex (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                    : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] joint duplex (capture/playback): ");
  result +=
      (snd_pcm_hw_params_is_joint_duplex (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                     : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports sample-resolution synchronized start: ");
  result +=
      (snd_pcm_hw_params_can_sync_start (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                    : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports disabling period wakeups: ");
  result +=
      (snd_pcm_hw_params_can_disable_period_wakeup (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                               : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports audio wallclock timestamps: ");
  result +=
      (snd_pcm_hw_params_supports_audio_wallclock_ts (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                                 : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result += ACE_TEXT_ALWAYS_CHAR ("significant bits: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << snd_pcm_hw_params_get_sbits (format_in);
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("FIFO size (frames): ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << snd_pcm_hw_params_get_fifo_size (format_in);
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  return result;
}

__u32
Stream_Device_Tools::ffmpegFormatToV4L2Format (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::v4l2FormatToffmpegFormat"));

  switch (format_in)
  {
    case AV_PIX_FMT_YUV420P:
      return V4L2_PIX_FMT_YUV420;
    case AV_PIX_FMT_YUYV422:
      return V4L2_PIX_FMT_YUYV;
    case AV_PIX_FMT_RGB24:
      return V4L2_PIX_FMT_RGB24;
    case AV_PIX_FMT_BGR24:
      return V4L2_PIX_FMT_BGR24;
    case AV_PIX_FMT_YUV422P:
      return V4L2_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUV444P:
      return V4L2_PIX_FMT_YUV444;
    case AV_PIX_FMT_YUV410P:
      return V4L2_PIX_FMT_YUV410;
    case AV_PIX_FMT_YUV411P:
      return V4L2_PIX_FMT_YUV411P;
    case AV_PIX_FMT_GRAY8:
      return V4L2_PIX_FMT_GREY;
//    case AV_PIX_FMT_MONOWHITE:
//    case AV_PIX_FMT_MONOBLACK:
    case AV_PIX_FMT_PAL8:
      return V4L2_PIX_FMT_PAL8;
//    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVJ422P:
      // *TODO*: libav doesn't specify a pixel format for MJPEG (it is a codec)
      return V4L2_PIX_FMT_MJPEG;
//    case AV_PIX_FMT_YUVJ444P:
//    case AV_PIX_FMT_XVMC_MPEG2_MC:
//    case AV_PIX_FMT_XVMC_MPEG2_IDCT:
//    case AV_PIX_FMT_XVMC:
    case AV_PIX_FMT_UYVY422:
      return V4L2_PIX_FMT_UYVY;
//    case AV_PIX_FMT_UYYVYY411:
//    case AV_PIX_FMT_BGR8:
//    case AV_PIX_FMT_BGR4:
//    case AV_PIX_FMT_BGR4_BYTE:
//    case AV_PIX_FMT_RGB8:
//    case AV_PIX_FMT_RGB4:
//    case AV_PIX_FMT_RGB4_BYTE:
    case AV_PIX_FMT_NV12:
      return V4L2_PIX_FMT_NV12;
    case AV_PIX_FMT_NV21:
      return V4L2_PIX_FMT_NV21;
    case AV_PIX_FMT_ARGB:
      return V4L2_PIX_FMT_ARGB32;
    case AV_PIX_FMT_RGBA:
      return V4L2_PIX_FMT_RGB32;
    case AV_PIX_FMT_ABGR:
      return V4L2_PIX_FMT_ABGR32;
    case AV_PIX_FMT_BGRA:
      return V4L2_PIX_FMT_BGR32;
    case AV_PIX_FMT_GRAY16BE:
      return V4L2_PIX_FMT_Y16_BE;
    case AV_PIX_FMT_GRAY16LE:
      return V4L2_PIX_FMT_Y16;
//    case AV_PIX_FMT_YUV440P:
//    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUVA420P:
      return V4L2_PIX_FMT_YUV420;
//    case AV_PIX_FMT_VDPAU_H264:
//    case AV_PIX_FMT_VDPAU_MPEG1:
//    case AV_PIX_FMT_VDPAU_MPEG2:
//    case AV_PIX_FMT_VDPAU_WMV3:
//    case AV_PIX_FMT_VDPAU_VC1:
//     case AV_PIX_FMT_RGB48BE:
//     case AV_PIX_FMT_RGB48LE:
     case AV_PIX_FMT_RGB565BE:
      return V4L2_PIX_FMT_RGB565X;
    case AV_PIX_FMT_RGB565LE:
     return V4L2_PIX_FMT_RGB565;
    case AV_PIX_FMT_RGB555BE:
     return V4L2_PIX_FMT_RGB555X;
    case AV_PIX_FMT_RGB555LE:
     return V4L2_PIX_FMT_RGB555;
//    case AV_PIX_FMT_BGR565BE:
//    case AV_PIX_FMT_BGR565LE:
//    case AV_PIX_FMT_BGR555BE:
//    case AV_PIX_FMT_BGR555LE:
//    case AV_PIX_FMT_VAAPI_MOCO:
//    case AV_PIX_FMT_VAAPI_IDCT:
//    case AV_PIX_FMT_VAAPI_VLD:
//    case AV_PIX_FMT_VAAPI:
//    case AV_PIX_FMT_YUV420P16LE:
//    case AV_PIX_FMT_YUV420P16BE:
//    case AV_PIX_FMT_YUV422P16LE:
//    case AV_PIX_FMT_YUV422P16BE:
//    case AV_PIX_FMT_YUV444P16LE:
//    case AV_PIX_FMT_YUV444P16BE:
//    case AV_PIX_FMT_VDPAU_MPEG4:
//    case AV_PIX_FMT_DXVA2_VLD:
    case AV_PIX_FMT_RGB444LE:
    case AV_PIX_FMT_RGB444BE:
      return V4L2_PIX_FMT_RGB444;
//    case AV_PIX_FMT_BGR444LE:
//    case AV_PIX_FMT_BGR444BE:
//    case AV_PIX_FMT_YA8:
//    case AV_PIX_FMT_Y400A:
//    case AV_PIX_FMT_GRAY8A:
//    case AV_PIX_FMT_BGR48BE:
//    case AV_PIX_FMT_BGR48LE:
//    case AV_PIX_FMT_YUV420P9BE:
//    case AV_PIX_FMT_YUV420P9LE:
//    case AV_PIX_FMT_YUV420P10BE:
//    case AV_PIX_FMT_YUV420P10LE:
//    case AV_PIX_FMT_YUV422P10BE:
//    case AV_PIX_FMT_YUV422P10LE:
//    case AV_PIX_FMT_YUV444P9BE:
//    case AV_PIX_FMT_YUV444P9LE:
//    case AV_PIX_FMT_YUV444P10BE:
//    case AV_PIX_FMT_YUV444P10LE:
//    case AV_PIX_FMT_YUV422P9BE:
//    case AV_PIX_FMT_YUV422P9LE:
//    case AV_PIX_FMT_VDA_VLD:
//    case AV_PIX_FMT_GBRP:
//    case AV_PIX_FMT_GBR24P:
//    case AV_PIX_FMT_GBRP9BE:
//    case AV_PIX_FMT_GBRP9LE:
//    case AV_PIX_FMT_GBRP10BE:
//    case AV_PIX_FMT_GBRP10LE:
//    case AV_PIX_FMT_GBRP16BE:
//    case AV_PIX_FMT_GBRP16LE:
//    case AV_PIX_FMT_YUVA422P:
//    case AV_PIX_FMT_YUVA444P:
//    case AV_PIX_FMT_YUVA420P9BE:
//    case AV_PIX_FMT_YUVA420P9LE:
//    case AV_PIX_FMT_YUVA422P9BE:
//    case AV_PIX_FMT_YUVA422P9LE:
//    case AV_PIX_FMT_YUVA444P9BE:
//    case AV_PIX_FMT_YUVA444P9LE:
//    case AV_PIX_FMT_YUVA420P10BE:
//    case AV_PIX_FMT_YUVA420P10LE:
//    case AV_PIX_FMT_YUVA422P10BE:
//    case AV_PIX_FMT_YUVA422P10LE:
//    case AV_PIX_FMT_YUVA444P10BE:
//    case AV_PIX_FMT_YUVA444P10LE:
//    case AV_PIX_FMT_YUVA420P16BE:
//    case AV_PIX_FMT_YUVA420P16LE:
//    case AV_PIX_FMT_YUVA422P16BE:
//    case AV_PIX_FMT_YUVA422P16LE:
//    case AV_PIX_FMT_YUVA444P16BE:
//    case AV_PIX_FMT_YUVA444P16LE:
//    case AV_PIX_FMT_VDPAU:
//    case AV_PIX_FMT_XYZ12LE:
//    case AV_PIX_FMT_XYZ12BE:
    case AV_PIX_FMT_NV16:
      return V4L2_PIX_FMT_NV16;
//    case AV_PIX_FMT_NV20LE:
//    case AV_PIX_FMT_NV20BE:
//    case AV_PIX_FMT_RGBA64BE:
//    case AV_PIX_FMT_RGBA64LE:
//    case AV_PIX_FMT_BGRA64BE:
//    case AV_PIX_FMT_BGRA64LE:
    case AV_PIX_FMT_YVYU422:
      return V4L2_PIX_FMT_YVYU;
//    case AV_PIX_FMT_VDA:
//    case AV_PIX_FMT_YA16BE:
//    case AV_PIX_FMT_YA16LE:
//    case AV_PIX_FMT_GBRAP:
//    case AV_PIX_FMT_GBRAP16BE:
//    case AV_PIX_FMT_GBRAP16LE:
//    case AV_PIX_FMT_QSV:
//    case AV_PIX_FMT_MMAL:
//    case AV_PIX_FMT_D3D11VA_VLD:
//    case AV_PIX_FMT_CUDA:
//    case AV_PIX_FMT_0RGB:
//    case AV_PIX_FMT_RGB0:
//    case AV_PIX_FMT_0BGR:
//    case AV_PIX_FMT_BGR0:
//    case AV_PIX_FMT_YUV420P12BE:
//    case AV_PIX_FMT_YUV420P12LE:
//    case AV_PIX_FMT_YUV420P14BE:
//    case AV_PIX_FMT_YUV420P14LE:
//    case AV_PIX_FMT_YUV422P12BE:
//    case AV_PIX_FMT_YUV422P12LE:
//    case AV_PIX_FMT_YUV422P14BE:
//    case AV_PIX_FMT_YUV422P14LE:
//    case AV_PIX_FMT_YUV444P12BE:
//    case AV_PIX_FMT_YUV444P12LE:
//    case AV_PIX_FMT_YUV444P14BE:
//    case AV_PIX_FMT_YUV444P14LE:
//    case AV_PIX_FMT_GBRP12BE:
//    case AV_PIX_FMT_GBRP12LE:
//    case AV_PIX_FMT_GBRP14BE:
//    case AV_PIX_FMT_GBRP14LE:
//    case AV_PIX_FMT_YUVJ411P:
    case AV_PIX_FMT_BAYER_BGGR8:
      return V4L2_PIX_FMT_SBGGR8;
    case AV_PIX_FMT_BAYER_RGGB8:
      return V4L2_PIX_FMT_SRGGB8;
    case AV_PIX_FMT_BAYER_GBRG8:
      return V4L2_PIX_FMT_SGBRG8;
    case AV_PIX_FMT_BAYER_GRBG8:
      return V4L2_PIX_FMT_SGRBG8;
    case AV_PIX_FMT_BAYER_BGGR16LE:
    case AV_PIX_FMT_BAYER_BGGR16BE:
      return V4L2_PIX_FMT_SBGGR16;
    case AV_PIX_FMT_BAYER_RGGB16LE:
    case AV_PIX_FMT_BAYER_RGGB16BE:
      return V4L2_PIX_FMT_SRGGB16;
    case AV_PIX_FMT_BAYER_GBRG16LE:
    case AV_PIX_FMT_BAYER_GBRG16BE:
      return V4L2_PIX_FMT_SGBRG16;
    case AV_PIX_FMT_BAYER_GRBG16LE:
    case AV_PIX_FMT_BAYER_GRBG16BE:
      return V4L2_PIX_FMT_SGRBG16;
////     case AV_PIX_FMT_XVMC:
//    case AV_PIX_FMT_YUV440P10LE:
//    case AV_PIX_FMT_YUV440P10BE:
//    case AV_PIX_FMT_YUV440P12LE:
//    case AV_PIX_FMT_YUV440P12BE:
//    case AV_PIX_FMT_AYUV64LE:
//    case AV_PIX_FMT_AYUV64BE:
//    case AV_PIX_FMT_VIDEOTOOLBOX:
//    case AV_PIX_FMT_P010LE:
//    case AV_PIX_FMT_P010BE:
//    case AV_PIX_FMT_GBRAP12BE:
//    case AV_PIX_FMT_GBRAP12LE:
//    case AV_PIX_FMT_GBRAP10BE:
//    case AV_PIX_FMT_GBRAP10LE:
//    case AV_PIX_FMT_MEDIACODEC:
//    case AV_PIX_FMT_GRAY12BE:
//    case AV_PIX_FMT_GRAY12LE:
//    case AV_PIX_FMT_GRAY10BE:
//    case AV_PIX_FMT_GRAY10LE:
//    case AV_PIX_FMT_P016LE:
//    case AV_PIX_FMT_P016BE:
//    case AV_PIX_FMT_D3D11:
//    case AV_PIX_FMT_GRAY9BE:
//    case AV_PIX_FMT_GRAY9LE:
//    case AV_PIX_FMT_GBRPF32BE:
//    case AV_PIX_FMT_GBRPF32LE:
//    case AV_PIX_FMT_GBRAPF32BE:
//    case AV_PIX_FMT_GBRAPF32LE:
//    case AV_PIX_FMT_DRM_PRIME:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ffmpeg pixel format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return 0;
}

enum AVPixelFormat
Stream_Device_Tools::v4l2FormatToffmpegFormat (__u32 format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::v4l2FormatToffmpegFormat"));

  switch (format_in)
  {
//    case V4L2_PIX_FMT_RGB332:
    case V4L2_PIX_FMT_RGB444:
      return AV_PIX_FMT_RGB444;
    case V4L2_PIX_FMT_RGB555:
      return AV_PIX_FMT_RGB555;
    case V4L2_PIX_FMT_RGB565:
      return AV_PIX_FMT_RGB565;
    case V4L2_PIX_FMT_RGB555X:
      return AV_PIX_FMT_RGB555BE;
    case V4L2_PIX_FMT_RGB565X:
      return AV_PIX_FMT_RGB565BE;
    case V4L2_PIX_FMT_BGR666:
      return AV_PIX_FMT_BGR555; // *TODO*: this is wrong
    case V4L2_PIX_FMT_BGR24:
      return AV_PIX_FMT_BGR24;
    case V4L2_PIX_FMT_RGB24:
      return AV_PIX_FMT_RGB24;
    case V4L2_PIX_FMT_BGR32:
      return AV_PIX_FMT_BGR32;
    case V4L2_PIX_FMT_RGB32:
      return AV_PIX_FMT_RGB32;
    case V4L2_PIX_FMT_GREY:
      return AV_PIX_FMT_GRAY8;
//    case V4L2_PIX_FMT_Y4:
//    case V4L2_PIX_FMT_Y6:
//    case V4L2_PIX_FMT_Y10:
//    case V4L2_PIX_FMT_Y12:
    case V4L2_PIX_FMT_Y16:
      return AV_PIX_FMT_GRAY16;
//    case V4L2_PIX_FMT_Y10BPACK:
    case V4L2_PIX_FMT_PAL8:
      return AV_PIX_FMT_PAL8;
//    case V4L2_PIX_FMT_UV8:
    case V4L2_PIX_FMT_YVU410:
      return AV_PIX_FMT_YUV410P; // *TODO*: this is wrong
    case V4L2_PIX_FMT_YVU420:
      return AV_PIX_FMT_YUV420P; // *TODO*: this is wrong
    case V4L2_PIX_FMT_YUYV:
      return AV_PIX_FMT_YUYV422;
//    case V4L2_PIX_FMT_YYUV:
    case V4L2_PIX_FMT_YVYU:
      return AV_PIX_FMT_YVYU422;
    case V4L2_PIX_FMT_UYVY:
      return AV_PIX_FMT_UYVY422;
//    case V4L2_PIX_FMT_VYUY:
    case V4L2_PIX_FMT_YUV422P:
      return AV_PIX_FMT_YUV422P;
    case V4L2_PIX_FMT_YUV411P:
      return AV_PIX_FMT_YUV411P;
    case V4L2_PIX_FMT_Y41P:
      return AV_PIX_FMT_YUV411P;
    case V4L2_PIX_FMT_YUV444:
      return AV_PIX_FMT_YUV444P;
//    case V4L2_PIX_FMT_YUV555:
//    case V4L2_PIX_FMT_YUV565:
//    case V4L2_PIX_FMT_YUV32:
    case V4L2_PIX_FMT_YUV410:
      return AV_PIX_FMT_YUV410P;
    case V4L2_PIX_FMT_YUV420:
      return AV_PIX_FMT_YUV420P;
//    case V4L2_PIX_FMT_HI240:
//    case V4L2_PIX_FMT_HM12:
//    case V4L2_PIX_FMT_M420:
    case V4L2_PIX_FMT_NV12:
      return AV_PIX_FMT_NV12;
    case V4L2_PIX_FMT_NV21:
      return AV_PIX_FMT_NV21;
    case V4L2_PIX_FMT_NV16:
      return AV_PIX_FMT_NV16;
//    case V4L2_PIX_FMT_NV61:
//    case V4L2_PIX_FMT_NV24:
//    case V4L2_PIX_FMT_NV42:
//    case V4L2_PIX_FMT_NV12M:
//    case V4L2_PIX_FMT_NV21M:
//    case V4L2_PIX_FMT_NV16M:
//    case V4L2_PIX_FMT_NV61M:
//    case V4L2_PIX_FMT_NV12MT:
//    case V4L2_PIX_FMT_NV12MT_16X16:
//    case V4L2_PIX_FMT_YUV420M:
//    case V4L2_PIX_FMT_YVU420M:
    case V4L2_PIX_FMT_SBGGR8:
      return AV_PIX_FMT_BAYER_BGGR8;
    case V4L2_PIX_FMT_SGBRG8:
      return AV_PIX_FMT_BAYER_GBRG8;
    case V4L2_PIX_FMT_SGRBG8:
      return AV_PIX_FMT_BAYER_GRBG8;
    case V4L2_PIX_FMT_SRGGB8:
      return AV_PIX_FMT_BAYER_RGGB8;
//    case V4L2_PIX_FMT_SBGGR10:
//    case V4L2_PIX_FMT_SGBRG10:
//    case V4L2_PIX_FMT_SGRBG10:
//    case V4L2_PIX_FMT_SRGGB10:
//    case V4L2_PIX_FMT_SBGGR12:
//    case V4L2_PIX_FMT_SGBRG12:
//    case V4L2_PIX_FMT_SGRBG12:
//    case V4L2_PIX_FMT_SRGGB12:
//    case V4L2_PIX_FMT_SBGGR10ALAW8:
//    case V4L2_PIX_FMT_SGBRG10ALAW8:
//    case V4L2_PIX_FMT_SGRBG10ALAW8:
//    case V4L2_PIX_FMT_SRGGB10ALAW8:
//    case V4L2_PIX_FMT_SBGGR10DPCM8:
//    case V4L2_PIX_FMT_SGBRG10DPCM8:
//    case V4L2_PIX_FMT_SGRBG10DPCM8:
//    case V4L2_PIX_FMT_SRGGB10DPCM8:
    case V4L2_PIX_FMT_SBGGR16:
      return AV_PIX_FMT_BAYER_BGGR16;
    case V4L2_PIX_FMT_MJPEG:
      // *NOTE*: "... MJPEG, or at least the MJPEG in AVIs having the MJPG
      //         fourcc, is restricted JPEG with a fixed -- and *omitted* --
      //         Huffman table. The JPEG must be YCbCr colorspace, it must be
      //         4:2:2, and it must use basic Huffman encoding, not arithmetic
      //         or progressive. . . . You can indeed extract the MJPEG frames
      //         and decode them with a regular JPEG decoder, but you have to
      //         prepend the DHT segment to them, or else the decoder won't
      //         have any idea how to decompress the data. The exact table
      //         necessary is given in the OpenDML spec. ..."
      // *TODO*: libav doesn't specify a pixel format for MJPEG (it is a codec)
      return AV_PIX_FMT_YUVJ422P;
//    case V4L2_PIX_FMT_JPEG:
//    case V4L2_PIX_FMT_DV:
//    case V4L2_PIX_FMT_MPEG:
//    case V4L2_PIX_FMT_H264:
//    case V4L2_PIX_FMT_H264_NO_SC:
//    case V4L2_PIX_FMT_H264_MVC:
//    case V4L2_PIX_FMT_H263:
//    case V4L2_PIX_FMT_MPEG1:
//    case V4L2_PIX_FMT_MPEG2:
//    case V4L2_PIX_FMT_MPEG4:
//    case V4L2_PIX_FMT_XVID:
//    case V4L2_PIX_FMT_VC1_ANNEX_G:
//    case V4L2_PIX_FMT_VC1_ANNEX_L:
//    case V4L2_PIX_FMT_VP8:
//    case V4L2_PIX_FMT_CPIA1:
//    case V4L2_PIX_FMT_WNVA:
//    case V4L2_PIX_FMT_SN9C10X:
//    case V4L2_PIX_FMT_SN9C20X_I420:
//    case V4L2_PIX_FMT_PWC1:
//    case V4L2_PIX_FMT_PWC2:
//    case V4L2_PIX_FMT_ET61X251:
//    case V4L2_PIX_FMT_SPCA501:
//    case V4L2_PIX_FMT_SPCA505:
//    case V4L2_PIX_FMT_SPCA508:
//    case V4L2_PIX_FMT_SPCA561:
//    case V4L2_PIX_FMT_PAC207:
//    case V4L2_PIX_FMT_MR97310A:
//    case V4L2_PIX_FMT_JL2005BCD:
//    case V4L2_PIX_FMT_SN9C2028:
//    case V4L2_PIX_FMT_SQ905C:
//    case V4L2_PIX_FMT_PJPG:
//    case V4L2_PIX_FMT_OV511:
//    case V4L2_PIX_FMT_OV518:
//    case V4L2_PIX_FMT_STV0680:
//    case V4L2_PIX_FMT_TM6000:
//    case V4L2_PIX_FMT_CIT_YYVYUY:
//    case V4L2_PIX_FMT_KONICA420:
//    case V4L2_PIX_FMT_JPGL:
//    case V4L2_PIX_FMT_SE401:
//    case V4L2_PIX_FMT_S5C_UYVY_JPG:
//    case V4L2_SDR_FMT_CU8:
//    case V4L2_SDR_FMT_CU16LE:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown v4l2 pixel format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return AV_PIX_FMT_NONE;
}
#endif // ACE_WIN32 || ACE_WIN64
