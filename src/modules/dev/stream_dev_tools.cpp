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

#include "stream_dev_tools.h"

#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "amvideo.h"
#include "d3d9.h"
#include "d3d9types.h"
#include "dvdmedia.h"
#include "dxva2api.h"
#include "strmif.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#include "windef.h"
#include "WinUser.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (LIBCAMERA_SUPPORT)
#include "libcamera/libcamera.h"
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

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
#include "stream_lib_directsound_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_tools.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_defines.h"
#endif // ACE_WIN32 || ACE_WIN64


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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
void
Stream_Device_Tools::id (const struct Stream_Device_Identifier& deviceIdentifier_in,
                         UINT& deviceIdentifier_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::id"));

  // initialize return value(s)
  deviceIdentifier_out = -1;

  switch (deviceIdentifier_in.identifierDiscriminator)
  {
    case Stream_Device_Identifier::ID:
    {
      deviceIdentifier_out = deviceIdentifier_in.identifier._id;
      break;
    }
    case Stream_Device_Identifier::GUID:
    {
      deviceIdentifier_out =
        Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (deviceIdentifier_in.identifier._guid);
      break;
    }
    case Stream_Device_Identifier::STRING:
    {
      deviceIdentifier_out =
        Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (Stream_MediaFramework_DirectSound_Tools::endpointIdToDirectSoundGUID (deviceIdentifier_in.identifier._string));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown discriminator (was: %d), aborting\n"),
                  deviceIdentifier_in.identifierDiscriminator));
      return;
    }
  } // end SWITCH
}

void
Stream_Device_Tools::id (const struct Stream_Device_Identifier& deviceIdentifier_in,
                         struct _GUID& deviceIdentifier_out,
                         bool isCapture_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::id"));

  // initialize return value(s)
  deviceIdentifier_out = GUID_NULL;

  switch (deviceIdentifier_in.identifierDiscriminator)
  {
    case Stream_Device_Identifier::ID:
    {
      deviceIdentifier_out =
        Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (deviceIdentifier_in.identifier._id,
                                                                                isCapture_in);
      break;
    }
    case Stream_Device_Identifier::GUID:
    {
      deviceIdentifier_out = deviceIdentifier_in.identifier._guid;
      break;
    }
    case Stream_Device_Identifier::STRING:
    {
      deviceIdentifier_out =
        Stream_MediaFramework_DirectSound_Tools::endpointIdToDirectSoundGUID (deviceIdentifier_in.identifier._string);
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown discriminator (was: %d), aborting\n"),
                  deviceIdentifier_in.identifierDiscriminator));
      return;
    }
  } // end SWITCH
}
#else
std::string
Stream_Device_Tools::getDefaultVideoCaptureDevice (bool useLibCamera_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getDefaultVideoCaptureDevice"));

  // initialize return value(s)
  std::string return_value;

  if (useLibCamera_in)
  {
#if defined (LIBCAMERA_SUPPORT)
    int result = -1;
    Stream_Device_List_t devices_a;
    libcamera::CameraManager* camera_manager_p = NULL;
    ACE_NEW_NORETURN (camera_manager_p,
                      libcamera::CameraManager ());
    ACE_ASSERT (camera_manager_p);
    result = camera_manager_p->start();
    if (result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to libcamera::CameraManager::start(): \"%m\", aborting\n")));
      goto error;
    } // end IF

    devices_a =
      Stream_Device_Tools::getVideoCaptureDevices (camera_manager_p);
    if (devices_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("found no video capture devices, aborting\n")));
      result = -1;
      goto error;
    } // end IF
    return_value = devices_a.front ().identifier;

error:
    camera_manager_p->stop ();
    delete camera_manager_p; camera_manager_p = NULL;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("useLibCamera_in specified, but LIBCAMERA_SUPPORT not set, aborting\n")));
    return return_value;
#endif // LIBCAMERA_SUPPORT
  } // end IF
  else
  {
    return_value =
      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
    return_value += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    return_value +=
      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
  } // end ELSE

  return return_value;
}

#if defined (LIBCAMERA_SUPPORT)
Stream_Device_List_t
Stream_Device_Tools::getVideoCaptureDevices (libcamera::CameraManager* manager_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getVideoCaptureDevices"));

  // sanity check(s)
  ACE_ASSERT (manager_in);

  // initialize return value(s)
  Stream_Device_List_t return_value;

  std::vector<std::shared_ptr<libcamera::Camera> > cameras_a =
    manager_in->cameras ();
  if (unlikely (cameras_a.empty ()))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no video capture devices found, continuing\n")));

  struct Stream_Device_Identifier device_identifier_s;
  for (std::vector<std::shared_ptr<libcamera::Camera> >::iterator iterator = cameras_a.begin ();
       iterator != cameras_a.end ();
       ++iterator)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("found video capture device \"%s\"\n"),
                ACE_TEXT ((*iterator)->id ().c_str ())));
    const libcamera::ControlList& properties_r = (*iterator)->properties ();
    ACE_ASSERT (properties_r.contains(libcamera::properties::Model));

    device_identifier_s.description =
        properties_r.get (libcamera::properties::Model);
    device_identifier_s.identifier = (*iterator)->id ();
    return_value.push_back (device_identifier_s);
  } // end FOR

  return return_value;
}

std::shared_ptr<libcamera::Camera>
Stream_Device_Tools::getCamera (libcamera::CameraManager* manager_in,
                                const std::string& deviceIdentifier_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getCamera"));

  // sanity check(s)
  ACE_ASSERT (manager_in);

  std::vector<std::shared_ptr<libcamera::Camera> > cameras_a =
    manager_in->cameras ();
  for (std::vector<std::shared_ptr<libcamera::Camera> >::iterator iterator = cameras_a.begin ();
       iterator != cameras_a.end ();
       ++iterator)
  {
    if ((*iterator)->id () == deviceIdentifier_in)
      return *iterator;
  } // end FOR

  return std::shared_ptr<libcamera::Camera> (nullptr);
}

Stream_MediaFramework_LibCamera_CaptureFormats_t
Stream_Device_Tools::getCaptureFormats (libcamera::Camera* camera_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getCaptureFormats"));

  // sanity check(s)
  ACE_ASSERT (camera_in);

  // initialize return value(s)
  Stream_MediaFramework_LibCamera_CaptureFormats_t return_value;

  libcamera::StreamRoles roles_a;
  roles_a.push_back (libcamera::StreamRole::Raw);
  std::unique_ptr<libcamera::CameraConfiguration> configuration_p =
    camera_in->generateConfiguration (roles_a);
  if (!configuration_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to generate configuration from role(s), aborting\n")));
    return return_value;
  } // end IF
  ACE_ASSERT (!configuration_p->empty ());
  for (libcamera::CameraConfiguration::iterator iterator = configuration_p->begin ();
       iterator != configuration_p->end ();
       ++iterator)
  {
    const libcamera::StreamFormats& formats_r = (*iterator).formats ();
    std::vector<libcamera::PixelFormat> formats_2 = formats_r.pixelformats ();
    for (std::vector<libcamera::PixelFormat>::iterator iterator_2 = formats_2.begin ();
         iterator_2 != formats_2.end ();
         ++iterator_2)
      return_value.push_back (std::make_pair (*iterator_2,
                                              (*iterator_2).toString ()));
  } // end FOR

  return return_value;
}

Common_Image_Resolutions_t
Stream_Device_Tools::getCaptureResolutions (libcamera::Camera* camera_in,
                                            const libcamera::PixelFormat& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getCaptureFormats"));

  // sanity check(s)
  ACE_ASSERT (camera_in);

  // initialize return value(s)
  Common_Image_Resolutions_t return_value;

  libcamera::StreamRoles roles_a;
  roles_a.push_back (libcamera::StreamRole::Raw);
  std::unique_ptr<libcamera::CameraConfiguration> configuration_p =
    camera_in->generateConfiguration (roles_a);
  if (!configuration_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to generate configuration from role(s), aborting\n")));
    return return_value;
  }
  ACE_ASSERT (!configuration_p->empty ());
  for (libcamera::CameraConfiguration::iterator iterator = configuration_p->begin ();
       iterator != configuration_p->end ();
       ++iterator)
  {
    const libcamera::StreamFormats& formats_r = (*iterator).formats ();
    std::vector<libcamera::Size> sizes = formats_r.sizes (format_in);
    for (std::vector<libcamera::Size>::iterator iterator_2 = sizes.begin ();
         iterator_2 != sizes.end ();
         ++iterator_2)
    {
      Common_Image_Resolution_t resolution_s;
      resolution_s.width = (*iterator_2).width;
      resolution_s.height = (*iterator_2).height;
      return_value.push_back (resolution_s);
    } // end FOR
  } // end FOR

  return return_value;
}

struct Stream_MediaFramework_LibCamera_MediaType
Stream_Device_Tools::defaultCaptureFormat (libcamera::Camera* camera_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::defaultCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (camera_in);

  // initialize return value(s)
  struct Stream_MediaFramework_LibCamera_MediaType return_value;

  libcamera::StreamRoles roles_a;
  roles_a.push_back (libcamera::StreamRole::Viewfinder);
  std::unique_ptr<libcamera::CameraConfiguration> configuration_p =
    camera_in->generateConfiguration (roles_a);
  if (!configuration_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to generate configuration from role(s), aborting\n")));
    return return_value;
  }
  ACE_ASSERT (!configuration_p->empty ());
  libcamera::StreamConfiguration& configuration_2 = configuration_p->at (0);
  return_value.format = configuration_2.pixelFormat;
  return_value.resolution = configuration_2.size;

  return return_value;
}

#if defined (FFMPEG_SUPPORT)
struct Stream_MediaFramework_FFMPEG_VideoMediaType
Stream_Device_Tools::convert (const struct Stream_MediaFramework_LibCamera_MediaType& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::convert"));

  struct Stream_MediaFramework_FFMPEG_VideoMediaType result;

  result.format =
      Stream_Device_Tools::libCameraFormatToffmpegFormat (format_in.format);
  result.frameRate.num = format_in.frameRateNumerator;
  result.frameRate.den = format_in.frameRateDenominator;
  result.resolution.width = format_in.resolution.width;
  result.resolution.height = format_in.resolution.height;

  return result;
}
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT

bool
Stream_Device_Tools::canOverlay (int fileDescriptor_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::canOverlay"));

  int result = -1;

  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_QUERYCAP,
                          &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_QUERYCAP,
                          &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
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
  if (unlikely (!entries.length ()))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no video capture devices found, continuing\n")));

  struct Stream_Device_Identifier device_identifier_s;
  struct v4l2_capability device_capabilities;
  std::string device_file_path;
  ACE_DIRENT* dirent_p = NULL;
  int file_descriptor = -1;
  int open_mode_i = O_RDONLY;
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

    file_descriptor =
        ACE_OS::open (ACE_TEXT_ALWAYS_CHAR (device_file_path.c_str ()),
                      open_mode_i);
    if (unlikely (file_descriptor == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::open(\"%s\",%d): \"%m\", continuing\n"),
                  ACE_TEXT (device_file_path.c_str ()),
                  open_mode_i));
      goto close;
    } // end IF

    ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
    result = ACE_OS::ioctl (file_descriptor,
                            VIDIOC_QUERYCAP,
                            &device_capabilities);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", continuing\n"),
                  file_descriptor, ACE_TEXT ("VIDIOC_QUERYCAP")));
      goto close;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: found video capture device \"%s\" (driver: \"%s\") on bus \"%s\"\n"),
                ACE_TEXT (device_file_path.c_str ()),
                ACE_TEXT (reinterpret_cast<char*> (device_capabilities.card)),
                ACE_TEXT (reinterpret_cast<char*> (device_capabilities.driver)),
                ACE_TEXT (reinterpret_cast<char*> (device_capabilities.bus_info))));

    device_identifier_s.description =
        ACE_TEXT_ALWAYS_CHAR (reinterpret_cast<char*> (device_capabilities.card));
    device_identifier_s.identifier = device_file_path;
    return_value.push_back (device_identifier_s);

close:
    result = ACE_OS::close (file_descriptor);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
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
                                            const Common_Image_Resolution_t& resolution_in,
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
    result = ACE_OS::ioctl (fileDescriptor_in,
                            VIDIOC_ENUM_FRAMEINTERVALS,
                            &frmivalenum_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", continuing\n"),
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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_TRY_FMT,
                          &format_s);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
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
    result = ACE_OS::ioctl (fileDescriptor_in,
                            VIDIOC_ENUM_FMT,
                            &fmtdesc_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FMT")));
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("video capture device (fd: %d) has format \"%s\" [%d]\n"),
                fileDescriptor_in,
                ACE_TEXT (reinterpret_cast<char*> (fmtdesc_s.description)),
                fmtdesc_s.pixelformat));

    return_value.push_back (std::make_pair (fmtdesc_s.pixelformat,
                                            reinterpret_cast<char*> (fmtdesc_s.description)));
  } // end FOR

  return return_value;
}

Common_Image_Resolutions_t
Stream_Device_Tools::getCaptureResolutions (int fileDescriptor_in,
                                            __u32 pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getCaptureResolutions"));

  // initialize return value(s)
  Common_Image_Resolutions_t return_value;

  // sanity check(s)
  ACE_ASSERT (fileDescriptor_in != -1);

  int result = -1;
  struct v4l2_frmsizeenum frmsizeenum_s;
  Common_Image_Resolution_t resolution_s;
  for (__u32 i = 0;
       ;
       ++i)
  {
    ACE_OS::memset (&frmsizeenum_s, 0, sizeof (struct v4l2_frmsizeenum));
    frmsizeenum_s.index = i;
    frmsizeenum_s.pixel_format = pixelFormat_in;
    result = ACE_OS::ioctl (fileDescriptor_in,
                            VIDIOC_ENUM_FRAMESIZES,
                            &frmsizeenum_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FRAMESIZES")));
    } // end IF
    ACE_ASSERT (frmsizeenum_s.pixel_format == pixelFormat_in);
    ACE_ASSERT (frmsizeenum_s.type == V4L2_FRMSIZE_TYPE_DISCRETE);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("video capture device (fd: %d, format: \"%s\") has resolution %dx%d\n"),
                fileDescriptor_in,
                ACE_TEXT (Stream_Device_Tools::formatToString (fileDescriptor_in, pixelFormat_in).c_str ()),
                frmsizeenum_s.discrete.width, frmsizeenum_s.discrete.height));
    resolution_s.width = frmsizeenum_s.discrete.width;
    resolution_s.height = frmsizeenum_s.discrete.height;
    return_value.push_back (resolution_s);
  } // end FOR

  return return_value;
}

Common_UI_Framerates_t
Stream_Device_Tools::getCaptureFramerates (int fileDescriptor_in,
                                           __u32 pixelFormat_in,
                                           const Common_Image_Resolution_t& resolution_in)
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
    result = ACE_OS::ioctl (fileDescriptor_in,
                            VIDIOC_ENUM_FRAMEINTERVALS,
                            &frmivalenum_s);
    if (unlikely (result == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FRAMEINTERVALS")));
    } // end IF
    ACE_ASSERT (frmivalenum_s.pixel_format == pixelFormat_in);
    ACE_ASSERT (frmivalenum_s.width == resolution_in.width);
    ACE_ASSERT (frmivalenum_s.height == resolution_in.height);
    ACE_ASSERT (frmivalenum_s.type == V4L2_FRMIVAL_TYPE_DISCRETE);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("video capture device (fd: %d, format: \"%s\", resolution: %dx%d) has framerate %d/%d\n"),
                fileDescriptor_in,
                ACE_TEXT (Stream_Device_Tools::formatToString (fileDescriptor_in, pixelFormat_in).c_str ()),
                resolution_in.width, resolution_in.height,
                frmivalenum_s.discrete.denominator, frmivalenum_s.discrete.numerator));
    return_value.push_back (frmivalenum_s.discrete.denominator);
  } // end FOR

  return return_value;
}

bool
Stream_Device_Tools::getDefaultCaptureFormat (const std::string& deviceIdentifier_in,
                                              struct Stream_MediaFramework_V4L_MediaType& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::getDefaultCaptureFormat"));

  // initialize return value(s)
  ACE_OS::memset (&format_out, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));

  // sanity check(s)
  ACE_ASSERT (!deviceIdentifier_in.empty ());

  // *NOTE*: use O_NONBLOCK with a reactor (v4l2_select()) or proactor
  //         (v4l2_poll()) for asynchronous operation
  // *TODO*: support O_NONBLOCK
  int open_mode = O_RDONLY;
  bool result = true;
  int result_2 = -1;
  int file_descriptor = ACE_OS::open (deviceIdentifier_in.c_str (),
                                      open_mode);
  if (unlikely (file_descriptor == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::open(\"%s\",%u): \"%m\", aborting\n"),
                ACE_TEXT (deviceIdentifier_in.c_str ()),
                open_mode));
    return false;
  } // end IF

  struct v4l2_format format_s;
  ACE_OS::memset (&format_s, 0, sizeof (struct v4l2_format));
  if (unlikely (!Stream_Device_Tools::getFormat (file_descriptor,
                                                 format_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::getFormat(%d), aborting\n"),
                file_descriptor));
    result = false;
    goto close;
  } // end IF
  ACE_ASSERT (format_s.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);
  format_out.format = format_s.fmt.pix;
  if (unlikely (!Stream_Device_Tools::getFrameRate (file_descriptor,
                                                    format_out.frameRate)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::getFrameRate(%d), aborting\n"),
                file_descriptor));
    ACE_OS::memset (&format_out, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
    result = false;
    goto close;
  } // end IF

close:
  result_2 = ACE_OS::close (file_descriptor);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", aborting\n"),
                file_descriptor));
    result = false;
  } // end IF

  return result;
}

#if defined (FFMPEG_SUPPORT)
struct Stream_MediaFramework_FFMPEG_VideoMediaType
Stream_Device_Tools::convert (const struct Stream_MediaFramework_V4L_MediaType& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::convert"));

  struct Stream_MediaFramework_FFMPEG_VideoMediaType result;

  result.format =
      Stream_MediaFramework_Tools::v4l2FormatToffmpegFormat (format_in.format.pixelformat);
  result.frameRate.num = format_in.frameRate.numerator;
  result.frameRate.den = format_in.frameRate.denominator;
  result.resolution.width = format_in.format.width;
  result.resolution.height = format_in.format.height;

  return result;
}
#endif // FFMPEG_SUPPORT

void
Stream_Device_Tools::dump (int fileDescriptor_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_Tools::dump"));

  int result = -1;

  // sanity check(s)
  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_QUERYCAP,
                          &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", returning\n"),
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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_REQBUFS,
                          &request_buffers);
  if (result == -1)
  { int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_REQBUFS")));
      return false;
    } // end IF
    goto no_support;
  } // end IF
  numberOfBuffers_inout = request_buffers.count;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocated %d device (fd: %d) buffer slots\n"),
              numberOfBuffers_inout,
              fileDescriptor_in));
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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_G_FBUF,
                          &framebuffer_s);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FBUF")));
      return false;
    } // end IF
    goto no_support;
  } //IF end

  // *TODO*: configure frame-buffer options

  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_S_FBUF,
                          &framebuffer_s);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_S_FBUF")));
    goto error;
  } // end IF

  // step2: set up output format / overlay window
  struct v4l2_format format;
  ACE_OS::memset (&format, 0, sizeof (struct v4l2_format));
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_G_FMT,
                          &format);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FMT")));
    goto error;
  } // end IF

  format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
  format.fmt.win = window_in;
  // *TODO*: configure format options

  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_S_FMT,
                          &format);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
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
    result_2 = ACE_OS::ioctl (fileDescriptor_in,
                              VIDIOC_QUERYBUF,
                              &buffer);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", continuing\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_QUERYBUF")));

    if (buffer.flags & V4L2_BUF_FLAG_DONE)
      ++done_out;
    if (buffer.flags & V4L2_BUF_FLAG_QUEUED)
      ++result;
  } // end FOR

  return result;
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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_G_FMT,
                          &format_s);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF
  format_s.fmt.pix.bytesperline = 0;
  format_s.fmt.pix.colorspace = 0;
  format_s.fmt.pix.field = 0;
  format_s.fmt.pix.height = format_in.height;
  format_s.fmt.pix.pixelformat = format_in.pixelformat;
  format_s.fmt.pix.priv = 0;
  format_s.fmt.pix.quantization = 0;
  format_s.fmt.pix.sizeimage = 0;
  format_s.fmt.pix.width = format_in.width;
  format_s.fmt.pix.xfer_func = 0;
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_S_FMT,
                          &format_s);
  if (result == -1)
  {// int error = ACE_OS::last_error (); ACE_UNUSED_ARG (error);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_S_FMT")));
    return false;
  } // end IF
  // validate result
  if ((format_s.fmt.pix.pixelformat != format_in.pixelformat) ||
      (format_s.fmt.pix.height != format_in.height)           ||
      (format_s.fmt.pix.width != format_in.width))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%d: failed to set capture format (was: %u;\"%s\"), aborting\n"),
                fileDescriptor_in,
                format_in.pixelformat, ACE_TEXT (Stream_Device_Tools::formatToString (fileDescriptor_in, format_in.pixelformat).c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: set capture format to %u;\"%s\" @ %ux%u\n"),
              fileDescriptor_in,
              format_in.pixelformat, ACE_TEXT (Stream_Device_Tools::formatToString (fileDescriptor_in, format_in.pixelformat).c_str ()),
              format_in.width, format_in.height));

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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_G_FMT,
                          &format_out);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF

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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_G_PARM,
                          &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_PARM")));
    return false;
  } // end IF
  if ((stream_parameters.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) == 0)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the device driver does not support frame interval settings, continuing\n")));

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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_G_PARM,
                          &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fileDescriptor_in, ACE_TEXT ("VIDIOC_G_PARM")));
    return false;
  } // end IF
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
  result = ACE_OS::ioctl (fileDescriptor_in,
                          VIDIOC_S_PARM,
                          &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
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

  int result_2 = -1;
  struct v4l2_fmtdesc fmtdesc_s;
  ACE_OS::memset (&fmtdesc_s, 0, sizeof (struct v4l2_fmtdesc));
  fmtdesc_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  do
  {
    result_2 = ACE_OS::ioctl (fileDescriptor_in,
                              VIDIOC_ENUM_FMT,
                              &fmtdesc_s);
    if (unlikely (result_2 == -1))
    {
      if (ACE_OS::last_error () == EINVAL) // 22
        break; // --> done
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                  fileDescriptor_in, ACE_TEXT ("VIDIOC_ENUM_FMT")));
      return result;
    } // end IF
    if (fmtdesc_s.pixelformat == pixelFormat_in)
    {
      result = reinterpret_cast<char*> (&fmtdesc_s.description);
      break;
    } // end IF
    ++fmtdesc_s.index;
  } while (true); // end WHILE
  if (unlikely (result.empty ()))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%d: failed to retrieve format string (id was: (%u), aborting\n"),
                fileDescriptor_in,
                pixelFormat_in));

  return result;
}
#endif // ACE_WIN32 || ACE_WIN64
