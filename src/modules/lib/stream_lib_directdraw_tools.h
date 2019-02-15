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

#ifndef STREAM_LIB_DIRECTDRAW_TOOLS_H
#define STREAM_LIB_DIRECTDRAW_TOOLS_H

#include <sdkddkver.h>

#include "common_defines.h"
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include <minwindef.h>
#else
#include <windef.h>
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#include <d3d9.h>
#include <d3dx9tex.h>
#include <dxva2api.h>

#include "ace/Global_Macros.h"

#include "common_image_common.h"

#include "stream_lib_defines.h"

// forward declarations
struct Stream_MediaFramework_Direct3D_Configuration;

class Stream_MediaFramework_DirectDraw_Tools
{
 public:
  static bool initialize (bool = true); // initialize COM ?
  static void finalize (bool = true); // finalize COM ?

  static struct _D3DDISPLAYMODE getDisplayMode (UINT,                              // adapter
                                                enum _D3DFORMAT,                   // format
                                                const Common_Image_Resolution_t&); // resolution
  // *IMPORTANT NOTE*: callers must Release() the handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  inline static IDirect3D9Ex* handle () { ACE_ASSERT (Stream_MediaFramework_DirectDraw_Tools::direct3DHandle); Stream_MediaFramework_DirectDraw_Tools::direct3DHandle->AddRef (); return Stream_MediaFramework_DirectDraw_Tools::direct3DHandle; }
#else
  inline static IDirect3D9* handle () { ACE_ASSERT (Stream_MediaFramework_DirectDraw_Tools::direct3DHandle); Stream_MediaFramework_DirectDraw_Tools::direct3DHandle->AddRef (); return Stream_MediaFramework_DirectDraw_Tools::direct3DHandle; }
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  static bool can (UINT,                              // adapter
                   enum _D3DFORMAT,                   // format
                   const Common_Image_Resolution_t&); // resolution

  static bool getDevice (struct Stream_MediaFramework_Direct3D_Configuration&, // in/out: Direct3D configuration
                         IDirect3DDeviceManager9*&,                            // return value: interface handle
                         UINT&);                                               // return value: reset token
  // *NOTE*: may be used to switch between windowed (aka 'desktop')/fullscreen
  //         modes
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool reset (IDirect3DDevice9Ex*,                                   // Direct3D device handle
#else
  static bool reset (IDirect3DDevice9*,                                     // Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                     struct Stream_MediaFramework_Direct3D_Configuration&); // in/out: Direct3D configuration

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool initializeDeviceManager (const IDirect3DDevice9Ex*, // Direct3D device handle
#else
  static bool initializeDeviceManager (const IDirect3DDevice9*,   // Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                       IDirect3DDeviceManager9*&, // return value: Direct3D device manager handle
                                       UINT&);                    // return value: reset token

  // *NOTE*: return value (if any) includes leading '.'
  static std::string toFilenameExtension (enum _D3DXIMAGE_FILEFORMAT);
  static enum _D3DFORMAT toFormat (REFGUID, // subtype
                                   enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectDraw_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectDraw_Tools (const Stream_MediaFramework_DirectDraw_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectDraw_Tools& operator= (const Stream_MediaFramework_DirectDraw_Tools&))

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static IDirect3D9Ex* direct3DHandle;
#else
  static IDirect3D9* direct3DHandle;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
};

#endif
