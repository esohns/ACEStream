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

#include "stream_lib_directdraw_tools.h"

// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#include <mfapi.h>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_lib_directdraw_common.h"

// initialize statics
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
IDirect3D9Ex*
#else
IDirect3D9*
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
Stream_MediaFramework_DirectDraw_Tools::direct3DHandle = NULL;

bool
Stream_MediaFramework_DirectDraw_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::initialize"));

  HRESULT result = E_FAIL;

  if (Stream_MediaFramework_DirectDraw_Tools::direct3DHandle)
    return true;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = Direct3DCreate9Ex (D3D_SDK_VERSION,
                              &Stream_MediaFramework_DirectDraw_Tools::direct3DHandle);
  if (unlikely (FAILED (result)))
#else
  Stream_MediaFramework_DirectDraw_Tools::direct3DHandle =
    Direct3DCreate9 (D3D_SDK_VERSION);
  if (unlikely (!Stream_MediaFramework_DirectDraw_Tools::direct3DHandle))
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Direct3DCreate9Ex(%d): \"%s\", aborting\n"),
                D3D_SDK_VERSION,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Direct3DCreate9(%d): \"%s\", aborting\n"),
                D3D_SDK_VERSION,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    return false;
  } // end IF
  ACE_ASSERT (Stream_MediaFramework_DirectDraw_Tools::direct3DHandle);

  return true;
}

void
Stream_MediaFramework_DirectDraw_Tools::finalize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::finalize"));

  if (likely (Stream_MediaFramework_DirectDraw_Tools::direct3DHandle))
  {
    Stream_MediaFramework_DirectDraw_Tools::direct3DHandle->Release (); Stream_MediaFramework_DirectDraw_Tools::direct3DHandle = NULL;
  } // end IF
}

struct _D3DDISPLAYMODE
Stream_MediaFramework_DirectDraw_Tools::getDisplayMode (UINT adapter_in,
                                                        enum _D3DFORMAT format_in,
                                                        const Common_Image_Resolution_t& resolution_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::getDisplayMode"));

  // initialize return value(s)
  struct _D3DDISPLAYMODE display_mode_s;

  // sanity check(s)
  IDirect3D9* interface_p = 
    Stream_MediaFramework_DirectDraw_Tools::handle ();
  ACE_ASSERT (interface_p);
  ACE_ASSERT (adapter_in <= (interface_p->GetAdapterCount () - 1));
  ACE_ASSERT (format_in == D3DFMT_A1R5G5B5 || format_in == D3DFMT_A2R10G10B10 || format_in == D3DFMT_A8R8G8B8 || format_in == D3DFMT_R5G6B5 || format_in == D3DFMT_X1R5G5B5 || format_in == D3DFMT_X8R8G8B8);

  UINT number_of_modes_i =
    interface_p->GetAdapterModeCount (adapter_in,
                                      format_in);
  if (unlikely (!number_of_modes_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9::GetAdapterModeCount(%u,%d), aborting\n"),
                adapter_in,
                format_in));
    ACE_OS::memset (&display_mode_s, 0, sizeof (struct _D3DDISPLAYMODE));
    return display_mode_s;
  } // end IF
  HRESULT result = E_FAIL;
  for (UINT i = 0;
       i < number_of_modes_i;
       ++i)
  {
    ACE_OS::memset (&display_mode_s, 0, sizeof (struct _D3DDISPLAYMODE));
    result = interface_p->EnumAdapterModes (adapter_in,
                                            format_in,
                                            i,
                                            &display_mode_s);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirect3D9::EnumAdapterModes(%u,%d,%d): \"%s\", aborting\n"),
                  adapter_in,
                  format_in,
                  i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    } // end IF
    ACE_ASSERT (display_mode_s.Format == format_in);
    if ((display_mode_s.Width  == resolution_in.cx) &&
        (display_mode_s.Height == resolution_in.cy))
      return display_mode_s;
  } // end FOR

  ACE_OS::memset (&display_mode_s, 0, sizeof (struct _D3DDISPLAYMODE));
  return display_mode_s;
}

bool
Stream_MediaFramework_DirectDraw_Tools::can (UINT adapter_in,
                                             enum _D3DFORMAT format_in,
                                             const Common_Image_Resolution_t& resolution_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::can"));

  struct _D3DDISPLAYMODE display_mode_s =
    Stream_MediaFramework_DirectDraw_Tools::getDisplayMode (adapter_in,
                                                            format_in,
                                                            resolution_in);
  if ((display_mode_s.Format == format_in) &&
      ((display_mode_s.Width  == resolution_in.cx) &&
       (display_mode_s.Height == resolution_in.cy)))
    return true;

  return false;
}

bool
Stream_MediaFramework_DirectDraw_Tools::getDevice (struct Stream_MediaFramework_Direct3D_Configuration& configuration_inout,
                                                   IDirect3DDeviceManager9*& deviceManager_out,
                                                   UINT& resetToken_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::getDevice"));

  HRESULT result = E_FAIL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IDirect3D9Ex* interface_p = NULL;
#else
  IDirect3D9* interface_p = NULL;
#endif

  // initialize return value(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
#else
  if (configuration_inout.handle)
  {
    result = configuration_inout.handle->GetDirect3D (&interface_p);
    ACE_ASSERT (SUCCEEDED (result) && interface_p);
  } // end IF
  else
#endif // _WIN32_WINNT_VISTA
    interface_p = Stream_MediaFramework_DirectDraw_Tools::handle ();
  ACE_ASSERT (interface_p);
  if (configuration_inout.handle)
  {
    configuration_inout.handle->Release (); configuration_inout.handle = NULL;
    configuration_inout.threadId = 0;
  } // end IF
  ACE_ASSERT (!deviceManager_out);
  ACE_ASSERT (!resetToken_out);

  struct _D3DCAPS9 d3d_capabilities_s;
  ACE_OS::memset (&d3d_capabilities_s, 0, sizeof (struct _D3DCAPS9));
  result =
    interface_p->GetDeviceCaps (configuration_inout.adapter,
                                configuration_inout.deviceType,
                                &d3d_capabilities_s);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::GetDeviceCaps(%d): \"%s\", aborting\n"),
                configuration_inout.adapter,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  if (d3d_capabilities_s.VertexProcessingCaps)
    configuration_inout.behaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
  else
    configuration_inout.behaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

  struct _D3DDISPLAYMODE d3d_display_mode_s;//, d3d_display_mode_2;
  ACE_OS::memset (&d3d_display_mode_s, 0, sizeof (struct _D3DDISPLAYMODE));
  result =
    interface_p->GetAdapterDisplayMode (configuration_inout.adapter,
                                        &d3d_display_mode_s);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::GetAdapterDisplayMode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  // sanity check(s)
  result =
    interface_p->CheckDeviceType (configuration_inout.adapter,
                                  configuration_inout.deviceType,
                                  d3d_display_mode_s.Format,
                                  configuration_inout.presentationParameters.BackBufferFormat,
                                  configuration_inout.presentationParameters.Windowed);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::CheckDeviceType(%d): \"%s\", aborting\n"),
                configuration_inout.deviceType,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  if (!configuration_inout.presentationParameters.Windowed)
  {
    Common_Image_Resolution_t resolution_s;
    resolution_s.cx =
      configuration_inout.presentationParameters.BackBufferWidth;
    resolution_s.cy =
      configuration_inout.presentationParameters.BackBufferHeight;
    if (unlikely (!Stream_MediaFramework_DirectDraw_Tools::can (configuration_inout.adapter,
                                                                configuration_inout.presentationParameters.BackBufferFormat,
                                                                resolution_s)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectDraw_Tools::can(%u,%u,%ux%u): \"%s\", aborting\n"),
                  configuration_inout.adapter,
                  configuration_inout.presentationParameters.BackBufferFormat,
                  resolution_s.cx, resolution_s.cy));
      goto error;
    } // end IF
  } // end IF
  if (configuration_inout.presentationParameters.EnableAutoDepthStencil)
  {
    result =
      interface_p->CheckDeviceFormat (configuration_inout.adapter,
                                      configuration_inout.deviceType,
                                      d3d_display_mode_s.Format,
                                      D3DUSAGE_DEPTHSTENCIL,
                                      D3DRTYPE_SURFACE,
                                      configuration_inout.presentationParameters.AutoDepthStencilFormat);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirect3D9Ex::CheckDeviceFormat(D3DUSAGE_DEPTHSTENCIL): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result =
      interface_p->CheckDepthStencilMatch (configuration_inout.adapter,
                                           configuration_inout.deviceType,
                                           d3d_display_mode_s.Format,
                                           configuration_inout.presentationParameters.BackBufferFormat,
                                           configuration_inout.presentationParameters.AutoDepthStencilFormat);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirect3D9Ex::CheckDepthStencilMatch(%d): \"%s\", aborting\n"),
                  configuration_inout.presentationParameters.AutoDepthStencilFormat,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF

  // *NOTE*: verify fullscreen destination alpha will work
  if ((configuration_inout.presentationParameters.SwapEffect == D3DSWAPEFFECT_FLIP) ||
      (configuration_inout.presentationParameters.SwapEffect == D3DSWAPEFFECT_DISCARD))
    if (unlikely (!(d3d_capabilities_s.Caps3 & D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD)))
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("driver cannot do alpha blending using this swap effect (was: %d), continuing\n"),
                  configuration_inout.presentationParameters.SwapEffect));

  result =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    interface_p->CreateDeviceEx (configuration_inout.adapter,                 // adapter
                                 configuration_inout.deviceType,              // device type
                                 configuration_inout.focusWindow,             // focus window handle
                                 configuration_inout.behaviorFlags,           // behavior flags
                                 &configuration_inout.presentationParameters, // presentation parameters
                                 (configuration_inout.presentationParameters.Windowed ? NULL 
                                                                                      : &configuration_inout.fullScreenDisplayMode),  // (fullscreen) display mode
                                 &configuration_inout.handle);                // return value: device handle
#else
    interface_p->CreateDevice (configuration_inout.adapter,                 // adapter
                               configuration_inout.deviceType,              // device type
                               configuration_inout.focusWindow,             // focus window handle
                               configuration_inout.behaviorFlags,           // behavior flags
                               &configuration_inout.presentationParameters, // presentation parameters
                               &configuration_inout.handle);                // return value: device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (unlikely (FAILED (result)))
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::CreateDeviceEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9::CreateDevice(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    goto error;
  } // end IF
  ACE_ASSERT (configuration_inout.handle);
  configuration_inout.threadId = ACE_OS::thr_self ();

  result = DXVA2CreateDirect3DDeviceManager9 (&resetToken_out,
                                              &deviceManager_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DXVA2CreateDirect3DDeviceManager9(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  result =
    deviceManager_out->ResetDevice (configuration_inout.handle,
                                    resetToken_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDeviceManager9::ResetDevice(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  interface_p->Release (); interface_p = NULL;

  return true;

error:
  if (configuration_inout.handle)
  {
    configuration_inout.handle->Release (); configuration_inout.handle = NULL;
  } // end IF
  if (deviceManager_out)
  {
    deviceManager_out->Release (); deviceManager_out = NULL;
  } // end IF
  resetToken_out = 0;
  if (interface_p)
    interface_p->Release ();

  return false;
}

bool
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
Stream_MediaFramework_DirectDraw_Tools::reset (IDirect3DDevice9Ex* deviceHandle_in,
#else
Stream_MediaFramework_DirectDraw_Tools::reset (IDirect3DDevice9* deviceHandle_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                               struct Stream_MediaFramework_Direct3D_Configuration& configuration_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::reset"));

  HRESULT result = E_FAIL;

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);
#if defined (_DEBUG)
  if (unlikely (!ACE_OS::thr_equal (ACE_OS::thr_self (), configuration_inout.threadId)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot invoke IDirect3DDevice9::Reset() from thread id: %t (expected %d), aborting\n"),
                configuration_inout.threadId));
    return false;
  } // end IF
#endif // _DEBUG

  // *NOTE*: "...When this method returns:
  // - BackBufferCount, BackBufferWidth, and BackBufferHeight are set to zero.
  // - BackBufferFormat is set to D3DFORMAT for windowed mode only; a
  //   full-screen mode must specify a format. ..."
  // *TODO*: this simply is not true; find out what is happening
  ACE_ASSERT (!configuration_inout.presentationParameters.Windowed ? (configuration_inout.presentationParameters.BackBufferFormat != D3DFMT_UNKNOWN) : true);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result =
    deviceHandle_in->ResetEx (&configuration_inout.presentationParameters,
                              &configuration_inout.fullScreenDisplayMode);
#else
  result = deviceHandle_in->Reset (&configuration_inout.presentationParameters);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (unlikely (FAILED (result)))
  {
    // *NOTE*: D3DERR_INVALIDCALL is returned when this is called from a
    //         different thread than that which allocated the device
    ACE_ASSERT (ACE_OS::thr_equal (ACE_OS::thr_self (), configuration_inout.threadId));
    // *NOTE*: "...If this method returns D3DERR_DEVICELOST or
    //         D3DERR_DEVICEHUNG then the application can only call
    //         IDirect3DDevice9Ex::ResetEx,
    //         IDirect3DDevice9Ex::CheckDeviceState or release the interface
    //         pointer; any other API call will cause an exception. ..."
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9Ex::ResetEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9::Reset(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    return false;
  } // end IF
  //ACE_ASSERT (!configuration_inout.presentationParameters.BackBufferCount && !configuration_inout.presentationParameters.BackBufferWidth && !configuration_inout.presentationParameters.BackBufferHeight);
  ACE_ASSERT (configuration_inout.presentationParameters.BackBufferFormat != D3DFMT_UNKNOWN);

  return true;
}

bool
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
Stream_MediaFramework_DirectDraw_Tools::initializeDeviceManager (const IDirect3DDevice9Ex* deviceHandle_in,
#else
Stream_MediaFramework_DirectDraw_Tools::initializeDeviceManager (const IDirect3DDevice9* deviceHandle_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                 IDirect3DDeviceManager9*& deviceManager_out,
                                                                 UINT& resetToken_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::initializeDeviceManager"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  if (deviceManager_out)
  {
    deviceManager_out->Release (); deviceManager_out = NULL;
  } // end IF
  ACE_ASSERT (resetToken_out == 0);

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);

  result =
    DXVA2CreateDirect3DDeviceManager9 (&resetToken_out,
                                       &deviceManager_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DXVA2CreateDirect3DDeviceManager9(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  result =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    deviceManager_out->ResetDevice (const_cast<IDirect3DDevice9Ex*> (deviceHandle_in),
#else
    deviceManager_out->ResetDevice (const_cast<IDirect3DDevice9*> (deviceHandle_in),
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                    resetToken_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDeviceManager9::ResetDevice(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (deviceManager_out)
  {
    deviceManager_out->Release (); deviceManager_out = NULL;
  } // end IF
  resetToken_out = 0;

  return false;

continue_:
  return true;
}

std::string
Stream_MediaFramework_DirectDraw_Tools::toFilenameExtension (enum _D3DXIMAGE_FILEFORMAT format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::toFilenameExtension"));

  std::string result;

  switch (format_in)
  {
    case D3DXIFF_BMP:
      result = ACE_TEXT_ALWAYS_CHAR (".bmp"); break;
    case D3DXIFF_JPG:
      result = ACE_TEXT_ALWAYS_CHAR (".jpg"); break;
    case D3DXIFF_TGA:
      result = ACE_TEXT_ALWAYS_CHAR (".tga"); break;
    case D3DXIFF_PNG:
      result = ACE_TEXT_ALWAYS_CHAR (".png"); break;
    case D3DXIFF_DDS:
      result = ACE_TEXT_ALWAYS_CHAR (".dds"); break;
    case D3DXIFF_PPM:
      result = ACE_TEXT_ALWAYS_CHAR (".ppm"); break;
    case D3DXIFF_DIB:
      result = ACE_TEXT_ALWAYS_CHAR (".dib"); break;
    case D3DXIFF_HDR:
      result = ACE_TEXT_ALWAYS_CHAR (".hdr"); break;
    case D3DXIFF_PFM:
      result = ACE_TEXT_ALWAYS_CHAR (".pfm"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown image file format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return result;
}

enum _D3DFORMAT
Stream_MediaFramework_DirectDraw_Tools::toFormat (REFGUID subType_in,
                                                  enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectDraw_Tools::toFormat"));

  enum _D3DFORMAT result = D3DFMT_UNKNOWN;

  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (subType_in, GUID_NULL));

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // *NOTE*: DirectShow encodes the enum _D3DFORMAT into some of its
      //         subtype GUIDs directly
      //         (see also: https://docs.microsoft.com/en-us/windows/desktop/medfound/video-subtype-guids)
      //         "...DirectShow also uses this system for most video subtypes,
      //         but not for uncompressed RGB formats. ..."
      // uncompressed RGB (no alpha)
      if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB1))
        ;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB4))
        ;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB8))
        return D3DFMT_P8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB555))
        return D3DFMT_X1R5G5B5;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB565))
        return D3DFMT_R5G6B5;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB24))
        return D3DFMT_R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32))
        return D3DFMT_X8R8G8B8;
      // uncompressed RGB (alpha)
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555))
        return D3DFMT_A4R4G4B4;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32))
        return D3DFMT_A8R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444))
        return D3DFMT_A4R4G4B4;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_A2R10G10B10))
        return D3DFMT_A2R10G10B10;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_A2B10G10R10))
        return D3DFMT_A2B10G10R10;
      // video mixing renderer (VMR-7)
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX7_RT))
        return D3DFMT_X8R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB16_D3D_DX7_RT))
        return D3DFMT_R5G6B5;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX7_RT))
        return D3DFMT_A8R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444_D3D_DX7_RT))
        return D3DFMT_A4R4G4B4;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555_D3D_DX7_RT))
        return D3DFMT_A4R4G4B4;
      // video mixing renderer (VMR-9)
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX9_RT))
        return D3DFMT_X8R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB16_D3D_DX9_RT))
        return D3DFMT_R5G6B5;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX9_RT))
        return D3DFMT_A8R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444_D3D_DX9_RT))
        return D3DFMT_A4R4G4B4;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555_D3D_DX9_RT))
        return D3DFMT_A4R4G4B4;
      if ((subType_in.Data2 == 0x0000)  && (subType_in.Data3 == 0x0010)  &&
          (subType_in.Data4[0] == 0x80) && (subType_in.Data4[1] == 0x00) && (subType_in.Data4[2] == 0x00) && (subType_in.Data4[3] == 0xAA) &&
          (subType_in.Data4[4] == 0x00) && (subType_in.Data4[5] == 0x38) && (subType_in.Data4[6] == 0x9B) && (subType_in.Data4[7] == 0x71))
        return static_cast<enum _D3DFORMAT> (subType_in.Data1);
      // *TODO*
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (result);
      ACE_NOTREACHED (return result;)
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // *NOTE*: MediaFoundation encodes the enum _D3DFORMAT into some of its
      //         subtype GUIDs directly
      //         (see also: https://docs.microsoft.com/en-us/windows/desktop/medfound/video-subtype-guids)
      if ((subType_in.Data2 == 0x0000)  && (subType_in.Data3 == 0x0010)  &&
          (subType_in.Data4[0] == 0x80) && (subType_in.Data4[1] == 0x00) && (subType_in.Data4[2] == 0x00) && (subType_in.Data4[3] == 0xAA) &&
          (subType_in.Data4[4] == 0x00) && (subType_in.Data4[5] == 0x38) && (subType_in.Data4[6] == 0x9B) && (subType_in.Data4[7] == 0x71))
        return static_cast<enum _D3DFORMAT> (subType_in.Data1);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (InlineIsEqualGUID (subType_in, MFVideoFormat_L8))
        return D3DFMT_L8;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_L16))
        return D3DFMT_L16;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_D16))
        return D3DFMT_D16;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      // *TODO*
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (result);
      ACE_NOTREACHED (return result;)
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
    }
  } // end SWITCH

  return result;
}
