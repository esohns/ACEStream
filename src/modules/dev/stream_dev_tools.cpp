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
#include <oleauto.h>

#include <dmoreg.h>
#include <dshow.h>
#include <dsound.h>
#include <dvdmedia.h>
#include <Dmodshow.h>
#include <evr.h>
#include <fourcc.h>
#include <ks.h>
#include <ksmedia.h>
 //#include <ksuuids.h>
#include <qedit.h>

#include <mfapi.h>
#include <mferror.h>
//#include <mftransform.h>

#include <wmcodecdsp.h>
#endif

#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
}
#endif /* __cplusplus */

#include "common_time_common.h"
#include "common_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"

// initialize statics
Stream_Module_Device_Tools::GUID2STRING_MAP_T Stream_Module_Device_Tools::Stream_FormatType2StringMap;
#endif

void
Stream_Module_Device_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initialize"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Device_DirectShow_Tools::initialize ();
  Stream_Module_Device_MediaFoundation_Tools::initialize ();

  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_None, ACE_TEXT_ALWAYS_CHAR ("None")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_VideoInfo, ACE_TEXT_ALWAYS_CHAR ("VideoInfo")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_VideoInfo2, ACE_TEXT_ALWAYS_CHAR ("VideoInfo2")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_WaveFormatEx, ACE_TEXT_ALWAYS_CHAR ("WaveFormatEx")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEGVideo, ACE_TEXT_ALWAYS_CHAR ("MPEGVideo")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEGStreams, ACE_TEXT_ALWAYS_CHAR ("MPEGStreams")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_DvInfo, ACE_TEXT_ALWAYS_CHAR ("DvInfo")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_525WSS, ACE_TEXT_ALWAYS_CHAR ("525WSS")));

  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEG2_VIDEO, ACE_TEXT_ALWAYS_CHAR ("MPEG2_VIDEO")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_VIDEOINFO2, ACE_TEXT_ALWAYS_CHAR ("VIDEOINFO2")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEG2Video, ACE_TEXT_ALWAYS_CHAR ("MPEG2Video")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_DolbyAC3, ACE_TEXT_ALWAYS_CHAR ("DolbyAC3")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEG2Audio, ACE_TEXT_ALWAYS_CHAR ("MPEG2Audio")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_DVD_LPCMAudio, ACE_TEXT_ALWAYS_CHAR ("DVD_LPCMAudio")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_UVCH264Video, ACE_TEXT_ALWAYS_CHAR ("UVCH264Video")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_JPEGImage, ACE_TEXT_ALWAYS_CHAR ("JPEGImage")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_Image, ACE_TEXT_ALWAYS_CHAR ("Image")));
#endif
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_Module_Device_Tools::isCompressed (REFGUID subType_in,
                                          REFGUID deviceCategory_in,
                                          bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::isCompressed"));

  if (deviceCategory_in == CLSID_AudioInputDeviceCategory)
    return isCompressedAudio (subType_in, useMediaFoundation_in);
  else if (deviceCategory_in == CLSID_VideoInputDeviceCategory)
    return isCompressedVideo (subType_in, useMediaFoundation_in);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
              ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (deviceCategory_in).c_str ())));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
}

bool
Stream_Module_Device_Tools::isCompressedAudio (REFGUID subType_in,
                                               bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::isCompressedAudio"));

  // *TODO*: this is probably incomplete
  if (useMediaFoundation_in)
    return ((subType_in != MFAudioFormat_PCM) &&
            (subType_in != MFAudioFormat_Float));

  return ((subType_in != MEDIASUBTYPE_PCM) &&
          (subType_in != MEDIASUBTYPE_IEEE_FLOAT));
}

bool
Stream_Module_Device_Tools::isCompressedVideo (REFGUID subType_in,
                                               bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::isCompressedVideo"));

  // *TODO*: this is probably incomplete
  return (!Stream_Module_Decoder_Tools::isChromaLuminance (subType_in, useMediaFoundation_in) &&
          !Stream_Module_Decoder_Tools::isRGB (subType_in, useMediaFoundation_in));
}

bool
Stream_Module_Device_Tools::getDirect3DDevice (const HWND windowHandle_in,
                                               const struct _AMMediaType& mediaType_in,
                                               IDirect3DDevice9Ex*& IDirect3DDevice9Ex_out,
                                               struct _D3DPRESENT_PARAMETERS_& presentationParameters_out,
                                               IDirect3DDeviceManager9*& IDirect3DDeviceManager9_out,
                                               UINT& resetToken_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getDirect3DDevice"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  if (IDirect3DDevice9Ex_out)
  {
    IDirect3DDevice9Ex_out->Release ();
    IDirect3DDevice9Ex_out = NULL;
  } // end IF
  ACE_OS::memset (&presentationParameters_out,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  ACE_ASSERT (resetToken_out == 0);

  struct tagVIDEOINFOHEADER* video_info_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_2 = NULL;
  if (mediaType_in.formattype == FORMAT_VideoInfo)
  {
    ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    video_info_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
    presentationParameters_out.BackBufferWidth =
      video_info_p->bmiHeader.biWidth;
    presentationParameters_out.BackBufferHeight =
      video_info_p->bmiHeader.biHeight;
  } // end IF
  else if (mediaType_in.formattype == FORMAT_VideoInfo2)
  {
    ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
    video_info_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_in.pbFormat);
    presentationParameters_out.BackBufferWidth =
      video_info_2->bmiHeader.biWidth;
    presentationParameters_out.BackBufferHeight =
      video_info_2->bmiHeader.biHeight;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return false;
  } // end ELSE

  IDirect3D9Ex* Direct3D9_p = NULL;
  result = Direct3DCreate9Ex (D3D_SDK_VERSION, &Direct3D9_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Direct3DCreate9Ex(%d): \"%s\", aborting\n"),
                D3D_SDK_VERSION,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  DWORD behavior_flags = (//D3DCREATE_ADAPTERGROUP_DEVICE          |
                          //D3DCREATE_DISABLE_DRIVER_MANAGEMENT    |
                          //D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX |
                          //D3DCREATE_DISABLE_PRINTSCREEN          |
                          //D3DCREATE_DISABLE_PSGP_THREADING       |
                          //D3DCREATE_ENABLE_PRESENTSTATS          |
                          D3DCREATE_FPU_PRESERVE                 |
                          D3DCREATE_HARDWARE_VERTEXPROCESSING    |
                          //D3DCREATE_MIXED_VERTEXPROCESSING       |
                          D3DCREATE_MULTITHREADED);//                |
                          //D3DCREATE_NOWINDOWCHANGES              |
                          //D3DCREATE_PUREDEVICE                   |
                          //D3DCREATE_SCREENSAVER                  |
                          //D3DCREATE_SOFTWARE_VERTEXPROCESSING);

  struct _D3DDISPLAYMODE d3d_display_mode;
  ACE_OS::memset (&d3d_display_mode,
                  0,
                  sizeof (struct _D3DDISPLAYMODE));
  result = Direct3D9_p->GetAdapterDisplayMode (D3DADAPTER_DEFAULT,
                                               &d3d_display_mode);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::GetAdapterDisplayMode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = Direct3D9_p->CheckDeviceType (D3DADAPTER_DEFAULT,
                                         D3DDEVTYPE_HAL,
                                         d3d_display_mode.Format,
                                         D3DFMT_X8R8G8B8,
                                         TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::CheckDeviceType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  //if (Mode == "fullscreen")
  //{
  //  presentationParameters_out.BackBufferWidth = x;
  //  presentationParameters_out.BackBufferHeight = y;
  //} // end IF
  //else if (Mode == "winmode")
  //{
  //presentationParameters_out.BackBufferWidth = video_info_p->bmiHeader.biWidth;
  //presentationParameters_out.BackBufferHeight =
  //  video_info_p->bmiHeader.biHeight;
  //} // end IF
  presentationParameters_out.BackBufferFormat = D3DFMT_X8R8G8B8;
  presentationParameters_out.BackBufferCount =
    MODULE_DEV_CAM_DIRECT3D_DEFAULT_BACK_BUFFERS;
  //presentationParameters_out.MultiSampleType = ;
  //presentationParameters_out.MultiSampleQuality = ;
  presentationParameters_out.SwapEffect = D3DSWAPEFFECT_FLIP;
  presentationParameters_out.hDeviceWindow = windowHandle_in;
  presentationParameters_out.Windowed = true;
  //presentationParameters_out.EnableAutoDepthStencil = ;
  //presentationParameters_out.AutoDepthStencilFormat = ;
  presentationParameters_out.Flags =
    (D3DPRESENTFLAG_LOCKABLE_BACKBUFFER            |
     //D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL           |
     D3DPRESENTFLAG_DEVICECLIP                     |
     D3DPRESENTFLAG_VIDEO);//                          |
     //D3DPRESENTFLAG_NOAUTOROTATE                   |
     //D3DPRESENTFLAG_UNPRUNEDMODE                   |
     //D3DPRESENTFLAG_OVERLAY_LIMITEDRGB             |
     //D3DPRESENTFLAG_OVERLAY_YCbCr_BT709            |
     //D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC            |
     //D3DPRESENTFLAG_RESTRICTED_CONTENT             |
     //D3DPRESENTFLAG_RESTRICT_SHARED_RESOURCE_DRIVER);
  //d3d_present_parameters.FullScreen_RefreshRateInHz = ;
  presentationParameters_out.PresentationInterval =
    D3DPRESENT_INTERVAL_IMMEDIATE;
  result =
    Direct3D9_p->CreateDeviceEx (D3DADAPTER_DEFAULT,          // adapter
                                 D3DDEVTYPE_HAL,              // device type
                                 windowHandle_in,             // focus window handle
                                 behavior_flags,              // behavior flags
                                 &presentationParameters_out, // presentation parameters
                                 NULL,                        // (fullscreen) display mode
                                 &IDirect3DDevice9Ex_out);    // return value: device handle
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::CreateDeviceEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  Direct3D9_p->Release ();
  Direct3D9_p = NULL;

  result = DXVA2CreateDirect3DDeviceManager9 (&resetToken_out,
                                              &IDirect3DDeviceManager9_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DXVA2CreateDirect3DDeviceManager9(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result =
    IDirect3DDeviceManager9_out->ResetDevice (IDirect3DDevice9Ex_out,
                                              resetToken_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDeviceManager9::ResetDevice(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (Direct3D9_p)
    Direct3D9_p->Release ();
  if (IDirect3DDevice9Ex_out)
  {
    IDirect3DDevice9Ex_out->Release ();
    IDirect3DDevice9Ex_out = NULL;
  } // end IF
  ACE_OS::memset (&presentationParameters_out,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  resetToken_out = 0;

  return false;

continue_:
  return true;
}
bool
Stream_Module_Device_Tools::initializeDirect3DManager (const IDirect3DDevice9Ex* IDirect3DDevice9Ex_in,
                                                       IDirect3DDeviceManager9*& IDirect3DDeviceManager9_out,
                                                       UINT& resetToken_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initializeDirect3DManager"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  ACE_ASSERT (resetToken_out == 0);

  // sanity check(s)
  ACE_ASSERT (IDirect3DDevice9Ex_in);

  result =
    DXVA2CreateDirect3DDeviceManager9 (&resetToken_out,
                                       &IDirect3DDeviceManager9_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DXVA2CreateDirect3DDeviceManager9(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result =
    IDirect3DDeviceManager9_out->ResetDevice (const_cast<IDirect3DDevice9Ex*> (IDirect3DDevice9Ex_in),
                                              resetToken_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDeviceManager9::ResetDevice(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  resetToken_out = 0;

  return false;

continue_:
  return true;
}

std::string
Stream_Module_Device_Tools::mediaFormatTypeToString (REFGUID GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::mediaFormatTypeToString"));

  std::string result;

  GUID2STRING_MAP_ITERATOR_T iterator =
    Stream_Module_Device_Tools::Stream_FormatType2StringMap.find (GUID_in);
  if (iterator == Stream_Module_Device_Tools::Stream_FormatType2StringMap.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (GUID_in).c_str ())));
    return result;
  } // end IF
  result = (*iterator).second;

  return result;
}
#else
void
Stream_Module_Device_Tools::dump (struct _snd_pcm* deviceHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::dump"));

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

  result = snd_pcm_hw_params_get_period_time_min (format_p,
                                                  &period_time_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_period_time_max (format_p,
                                                  &period_time_max, &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: available period times: %u-%u (us)...\n"),
              ACE_TEXT (snd_pcm_name (deviceHandle_in)),
              period_time_min, period_time_max));
  result = snd_pcm_hw_params_get_period_size_min (format_p,
                                                  &period_size_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_period_size_max (format_p,
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

  result = snd_pcm_hw_params_get_buffer_time_min (format_p,
                                                  &buffer_time_min, &subunit_direction);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_buffer_time_max (format_p,
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
Stream_Module_Device_Tools::canOverlay (int fd_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::canOverlay"));

  int result = -1;

  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return false;
  } // end IF

  return (device_capabilities.device_caps & V4L2_CAP_VIDEO_OVERLAY);
}
bool
Stream_Module_Device_Tools::canStream (int fd_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::canStream"));

  int result = -1;

  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return false;
  } // end IF

  return (device_capabilities.device_caps & V4L2_CAP_STREAMING);
}
void
Stream_Module_Device_Tools::dump (int fd_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::dump"));

  int result = -1;

  // sanity check(s)
  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", returning\n"),
                fd_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
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
              fd_in,
              ACE_TEXT (device_capabilities.driver),
              ACE_TEXT (device_capabilities.card),
              ACE_TEXT (device_capabilities.bus_info),
              ACE_TEXT (converter.str ().c_str ()),
              device_capabilities.capabilities,
              device_capabilities.device_caps,
              device_capabilities.reserved[0],device_capabilities.reserved[1],device_capabilities.reserved[2]));
}

std::string
Stream_Module_Device_Tools::getALSADeviceName (enum _snd_pcm_stream direction_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getALSADeviceName"));

  std::string result_string;

  // sanity check(s)
  ACE_ASSERT ((direction_in == SND_PCM_STREAM_CAPTURE) ||
              (direction_in == SND_PCM_STREAM_PLAYBACK));

  void** hints_p = NULL;
  int result =
      snd_device_name_hint (-1,
                            ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_ALSA_PCM_INTERFACE_NAME),
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
    free (string_p);
    string_p = NULL;

    // filter hardware devices
    device_type = hint_string;
    position = hint_string.find_first_of (':');
    if (position != std::string::npos)
      device_type = device_type.substr (0, position);
    if (ACE_OS::strcmp (device_type.c_str (),
                        (direction_in == SND_PCM_STREAM_PLAYBACK) ? ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_ALSA_DEVICE_PLAYBACK_PREFIX)
                                                                  : ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_ALSA_DEVICE_CAPTURE_PREFIX)))
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
//    free (string_p);
//    string_p = NULL;
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
Stream_Module_Device_Tools::initializeCapture (int fd_in,
                                               v4l2_memory method_in,
                                               __u32& numberOfBuffers_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initializeCapture"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (Stream_Module_Device_Tools::canStream (fd_in));

  struct v4l2_requestbuffers request_buffers;
  ACE_OS::memset (&request_buffers, 0, sizeof (struct v4l2_requestbuffers));
  request_buffers.count = numberOfBuffers_inout;
  request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request_buffers.memory = method_in;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_REQBUFS,
                       &request_buffers);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  fd_in, ACE_TEXT ("VIDIOC_REQBUFS")));
      return false;
    } // end IF
    goto no_support;
  } // end IF
  numberOfBuffers_inout = request_buffers.count;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocated %d device buffer slots...\n"),
              numberOfBuffers_inout));

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("device (fd was: %d) does not support streaming method (was: %d), aborting\n"),
              fd_in, method_in));

  return false;
}
bool
Stream_Module_Device_Tools::initializeOverlay (int fd_in,
                                               const struct v4l2_window& window_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initializeOverlay"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (Stream_Module_Device_Tools::canOverlay (fd_in));

  // step1: set up frame-buffer (if necessary)
  struct v4l2_framebuffer framebuffer;
  ACE_OS::memset (&framebuffer, 0, sizeof (struct v4l2_framebuffer));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_FBUF,
                       &framebuffer);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  fd_in, ACE_TEXT ("VIDIOC_G_FBUF")));
      return false;
    } // end IF
    goto no_support;
  } //IF end

  // *TODO*: configure frame-buffer options

  result = v4l2_ioctl (fd_in,
                       VIDIOC_S_FBUF,
                       &framebuffer);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_FBUF")));
    goto error;
  } //

  // step2: set up output format / overlay window
  struct v4l2_format format;
  ACE_OS::memset (&format, 0, sizeof (struct v4l2_format));

  format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
  format.fmt.win = window_in;
  // *TODO*: configure format options

  result = v4l2_ioctl (fd_in,
                       VIDIOC_S_FMT,
                       &format);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_FMT")));
    goto error;
  } //
  // *TODO*: verify that format now contains the requested configuration

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("device (was: %d) does not support overlays, aborting\n"),
              fd_in));
error:
  return false;
}

unsigned int
Stream_Module_Device_Tools::queued (int fd_in,
                                    unsigned int numberOfBuffers_in,
                                    unsigned int& done_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::queued"));

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
    result_2 = v4l2_ioctl (fd_in,
                           VIDIOC_QUERYBUF,
                           &buffer);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  fd_in, ACE_TEXT ("VIDIOC_QUERYBUF")));

    if (buffer.flags & V4L2_BUF_FLAG_DONE)
      ++done_out;
    if (buffer.flags & V4L2_BUF_FLAG_QUEUED)
      ++result;
  } // end FOR

  return result;
}

bool
Stream_Module_Device_Tools::setFormat (struct _snd_pcm* deviceHandle_in,
                                       const Stream_Module_Device_ALSAConfiguration& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setFormat"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);

  struct _snd_pcm_hw_params* format_p = NULL;
  Stream_Module_Device_ALSAConfiguration& format_s =
      const_cast<Stream_Module_Device_ALSAConfiguration&> (format_in);
  int subunit_direction = 0;

  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = snd_pcm_hw_params_any (deviceHandle_in, format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_access (deviceHandle_in, format_p,
                                         format_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_format (deviceHandle_in, format_p,
                                         format_in.format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
      snd_pcm_hw_params_set_channels (deviceHandle_in, format_p,
                                      format_in.channels);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_channels(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_rate_resample (deviceHandle_in, format_p,
                                                0);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_rate_resample(0): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_rate_near (deviceHandle_in, format_p,
                                       &format_s.rate,
                                       &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_rate_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
      snd_pcm_hw_params_set_period_time_near (deviceHandle_in, format_p,
                                              &format_s.periodTime,
                                              &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_period_time_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_period_size_near (deviceHandle_in, format_p,
                                              &format_s.periodSize,
                                              &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_period_size_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_periods_near (deviceHandle_in, format_p,
                                          &format_s.periods,
                                          &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_periods_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
      snd_pcm_hw_params_set_buffer_time_near (deviceHandle_in, format_p,
                                              &format_s.bufferTime,
                                              &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_buffer_time_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_buffer_size_near (deviceHandle_in, format_p,
                                              &format_s.bufferSize);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_buffer_size_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params (deviceHandle_in,
                              format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (deviceHandle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  snd_pcm_hw_params_free (format_p);

  return true;

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);

  return false;
}
bool
Stream_Module_Device_Tools::getFormat (struct _snd_pcm* deviceHandle_in,
                                       Stream_Module_Device_ALSAConfiguration& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getFormat"));

  int result = -1;
//  bool free_format = false;

  // initialize return value(s)
  ACE_OS::memset (&format_out,
                  0,
                  sizeof (Stream_Module_Device_ALSAConfiguration));

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);

  struct _snd_pcm_hw_params* format_p = NULL;
  unsigned int sample_rate_numerator, sample_rate_denominator;
  int subunit_direction = 0;
//  unsigned int rate_resample;

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
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
                                      format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_current(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  ACE_ASSERT (format_p);

  result = snd_pcm_hw_params_get_access (format_p,
                                         &format_out.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_format (format_p,
                                         &format_out.format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_subformat (format_p,
                                            &format_out.subFormat);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_subformat(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_channels (format_p,
                                           &format_out.channels);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_rate_numden (format_p,
                                              &sample_rate_numerator,
                                              &sample_rate_denominator);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_numden(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  format_out.rate = sample_rate_numerator;
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

  result = snd_pcm_hw_params_get_period_time (format_p,
                                              &format_out.periodTime,
                                              &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_period_size (format_p,
                                              &format_out.periodSize,
                                              &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_periods (format_p,
                                          &format_out.periods,
                                          &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_periods(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_buffer_time (format_p,
                                              &format_out.bufferTime,
                                              &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_buffer_size (format_p,
                                              &format_out.bufferSize);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  snd_pcm_hw_params_free (format_p);

  return true;

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);

  return false;
}

bool
Stream_Module_Device_Tools::setFormat (int fd_in,
                                       const struct v4l2_format& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setFormat"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);
  ACE_ASSERT (format_in.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  struct v4l2_format format_s;
  ACE_OS::memset (&format_s, 0, sizeof (format_s));
  format_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_FMT,
                       &format_s);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF
  format_s.fmt.pix.pixelformat = format_in.fmt.pix.pixelformat;
  format_s.fmt.pix.width = format_in.fmt.pix.width;
  format_s.fmt.pix.height = format_in.fmt.pix.height;

  result = v4l2_ioctl (fd_in,
                       VIDIOC_S_FMT,
                       &format_s);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_FMT")));
    return false;
  } // end IF

  return true;
}
bool
Stream_Module_Device_Tools::getFormat (int fd_in,
                                       struct v4l2_format& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getFormat"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);

  int result = -1;
  ACE_OS::memset (&format_out, 0, sizeof (struct v4l2_format));
  format_out.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_FMT,
                       &format_out);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF
//  ACE_ASSERT (format_out.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  return true;
}
bool
Stream_Module_Device_Tools::getFrameRate (int fd_in,
                                          struct v4l2_fract& frameRate_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getFrameRate"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);

  // initialize return value(s)
  ACE_OS::memset (&frameRate_out, 0, sizeof (struct v4l2_fract));

  int result = -1;
  struct v4l2_streamparm stream_parameters;
  ACE_OS::memset (&stream_parameters, 0, sizeof (struct v4l2_streamparm));
  stream_parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_PARM")));
    return false;
  } // end IF
  if ((stream_parameters.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) == 0)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the device driver does not support frame interval settings, continuing\n")));

  //  ACE_ASSERT (stream_parameters.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  // *NOTE*: the frame rate is the reciprocal value of the time-per-frame
  //         interval
  frameRate_out.numerator =
      stream_parameters.parm.capture.timeperframe.denominator;
  frameRate_out.numerator =
      stream_parameters.parm.capture.timeperframe.numerator;

  return true;
}
bool
Stream_Module_Device_Tools::setFrameRate (int fd_in,
                                          const struct v4l2_fract& frameRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setFrameRate"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);

  int result = -1;
  struct v4l2_streamparm stream_parameters;
  ACE_OS::memset (&stream_parameters, 0, sizeof (struct v4l2_streamparm));
  stream_parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_PARM")));
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
  result = v4l2_ioctl (fd_in,
                       VIDIOC_S_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_PARM")));
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
Stream_Module_Device_Tools::formatToString (uint32_t format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::formatToString"));

  std::string result;

  return result;
}
std::string
Stream_Module_Device_Tools::formatToString (const struct _snd_pcm_hw_params* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::formatToString"));

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

enum AVPixelFormat
Stream_Module_Device_Tools::v4l2FormatToffmpegFormat (__u32 format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::v4l2FormatToffmpegFormat"));

  enum AVPixelFormat result = AV_PIX_FMT_NONE;

  switch (format_in)
  {
//    case V4L2_PIX_FMT_RGB332:
    case V4L2_PIX_FMT_RGB444:
      result = AV_PIX_FMT_RGB444; break;
    case V4L2_PIX_FMT_RGB555:
      result = AV_PIX_FMT_RGB555; break;
    case V4L2_PIX_FMT_RGB565:
      result = AV_PIX_FMT_RGB565; break;
    case V4L2_PIX_FMT_RGB555X:
      result = AV_PIX_FMT_RGB555BE; break;
    case V4L2_PIX_FMT_RGB565X:
      result = AV_PIX_FMT_RGB565BE; break;
    case V4L2_PIX_FMT_BGR666:
      result = AV_PIX_FMT_BGR555; break; // *TODO*: this is wrong
    case V4L2_PIX_FMT_BGR24:
      result = AV_PIX_FMT_BGR24; break;
    case V4L2_PIX_FMT_RGB24:
      result = AV_PIX_FMT_RGB24; break;
    case V4L2_PIX_FMT_BGR32:
      result = AV_PIX_FMT_BGR32; break;
    case V4L2_PIX_FMT_RGB32:
      result = AV_PIX_FMT_RGB32; break;
    case V4L2_PIX_FMT_GREY:
      result = AV_PIX_FMT_GRAY8; break;
//    case V4L2_PIX_FMT_Y4:
//    case V4L2_PIX_FMT_Y6:
//    case V4L2_PIX_FMT_Y10:
//    case V4L2_PIX_FMT_Y12:
    case V4L2_PIX_FMT_Y16:
      result = AV_PIX_FMT_GRAY16; break;
//    case V4L2_PIX_FMT_Y10BPACK:
    case V4L2_PIX_FMT_PAL8:
      result = AV_PIX_FMT_PAL8; break;
//    case V4L2_PIX_FMT_UV8:
    case V4L2_PIX_FMT_YVU410:
      result = AV_PIX_FMT_YUV410P; break; // *TODO*: this is wrong
    case V4L2_PIX_FMT_YVU420:
      result = AV_PIX_FMT_YUV420P; break; // *TODO*: this is wrong
    case V4L2_PIX_FMT_YUYV:
      result = AV_PIX_FMT_YUYV422; break;
//    case V4L2_PIX_FMT_YYUV:
    case V4L2_PIX_FMT_YVYU:
      result = AV_PIX_FMT_YVYU422; break;
    case V4L2_PIX_FMT_UYVY:
      result = AV_PIX_FMT_UYVY422; break;
//    case V4L2_PIX_FMT_VYUY:
    case V4L2_PIX_FMT_YUV422P:
      result = AV_PIX_FMT_YUV422P; break;
    case V4L2_PIX_FMT_YUV411P:
      result = AV_PIX_FMT_YUV411P; break;
    case V4L2_PIX_FMT_Y41P:
      result = AV_PIX_FMT_YUV411P; break;
    case V4L2_PIX_FMT_YUV444:
      result = AV_PIX_FMT_YUV444P; break;
//    case V4L2_PIX_FMT_YUV555:
//    case V4L2_PIX_FMT_YUV565:
//    case V4L2_PIX_FMT_YUV32:
    case V4L2_PIX_FMT_YUV410:
      result = AV_PIX_FMT_YUV410P; break;
    case V4L2_PIX_FMT_YUV420:
      result = AV_PIX_FMT_YUV420P; break;
//    case V4L2_PIX_FMT_HI240:
//    case V4L2_PIX_FMT_HM12:
//    case V4L2_PIX_FMT_M420:
    case V4L2_PIX_FMT_NV12:
      result = AV_PIX_FMT_NV12; break;
    case V4L2_PIX_FMT_NV21:
      result = AV_PIX_FMT_NV21; break;
    case V4L2_PIX_FMT_NV16:
      result = AV_PIX_FMT_NV16; break;
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
      result = AV_PIX_FMT_BAYER_BGGR8; break;
    case V4L2_PIX_FMT_SGBRG8:
      result = AV_PIX_FMT_BAYER_GBRG8; break;
    case V4L2_PIX_FMT_SGRBG8:
      result = AV_PIX_FMT_BAYER_GRBG8; break;
    case V4L2_PIX_FMT_SRGGB8:
      result = AV_PIX_FMT_BAYER_RGGB8; break;
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
      result = AV_PIX_FMT_BAYER_BGGR16; break;
//    case V4L2_PIX_FMT_MJPEG:
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

  return result;
}
#endif
