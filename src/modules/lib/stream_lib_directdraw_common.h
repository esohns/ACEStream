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

#ifndef STREAM_LIB_DIRECTDRAW_COMMON_H
#define STREAM_LIB_DIRECTDRAW_COMMON_H

#include <d3d9.h>
#include <d3d9types.h>

#include "ace/OS.h"
#include "ace/OS_NS_Thread.h"
#include "ace/Synch_Traits.h"
#include "ace/Thread_Mutex.h"

#include "common_defines.h"

#include "stream_lib_defines.h"
#include "stream_lib_directdraw_tools.h"

struct Stream_MediaFramework_Direct3D_Configuration
{
  Stream_MediaFramework_Direct3D_Configuration ()
   : adapter (D3DADAPTER_DEFAULT)
   , behaviorFlags (0)
   , deviceType (D3DDEVTYPE_HAL)
   , focusWindow (NULL)
   , handle (NULL)
   , lock ()
   , presentationParameters ()
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
   , fullScreenDisplayMode ()
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
   //, usage (0)
   , threadId (0)
  {
    behaviorFlags = //D3DCREATE_ADAPTERGROUP_DEVICE          |
                    //D3DCREATE_DISABLE_DRIVER_MANAGEMENT    |
                    //D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX |
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                    //D3DCREATE_DISABLE_PRINTSCREEN          |
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                    //D3DCREATE_DISABLE_PSGP_THREADING       |
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
#if defined (_DEBUG)
                    D3DCREATE_ENABLE_PRESENTSTATS |
#endif // _DEBUG
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                    D3DCREATE_FPU_PRESERVE                   |
      //D3DCREATE_HARDWARE_VERTEXPROCESSING                  |
      //D3DCREATE_MIXED_VERTEXPROCESSING                     |
      //D3DCREATE_SOFTWARE_VERTEXPROCESSING                  |
                    D3DCREATE_MULTITHREADED//                  |
      //D3DCREATE_NOWINDOWCHANGES                            |
      //D3DCREATE_PUREDEVICE                                 |
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                    | D3DCREATE_SCREENSAVER
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                    ;

    ACE_OS::memset (&presentationParameters, 0, sizeof (struct _D3DPRESENT_PARAMETERS_));
    //presentationParameters.BackBufferWidth = 0;
    //presentationParameters.BackBufferHeight = 0;
    presentationParameters.BackBufferFormat =
      STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT;
    presentationParameters.BackBufferCount =
      STREAM_LIB_DIRECTDRAW_3D_DEFAULT_BACK_BUFFERS;
    presentationParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
    //presentationParameters.MultiSampleQuality = 0;
    presentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    //presentationParameters.hDeviceWindow = NULL;
    presentationParameters.Windowed = TRUE;
    presentationParameters.EnableAutoDepthStencil = FALSE;
    presentationParameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    presentationParameters.Flags =
      (D3DPRESENTFLAG_DEVICECLIP           | // "not valid with D3DSWAPEFFECT_FLIPEX"
       //D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL | // "illegal for all lockable formats"
       D3DPRESENTFLAG_LOCKABLE_BACKBUFFER  |
       //D3DPRESENTFLAG_NOAUTOROTATE         |
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
       D3DPRESENTFLAG_UNPRUNEDMODE         |
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
       D3DPRESENTFLAG_VIDEO);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
       //D3DPRESENTFLAG_OVERLAY_LIMITEDRGB   |
       //D3DPRESENTFLAG_OVERLAY_YCbCr_BT709  |
       //D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC  |
       //D3DPRESENTFLAG_RESTRICTED_CONTENT   |
       //D3DPRESENTFLAG_RESTRICT_SHARED_RESOURCE_DRIVER);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    //presentationParameters.FullScreen_RefreshRateInHz = 0;
    // *NOTE*: to prevent tearing: D3DPRESENT_INTERVAL_DEFAULT (i.e. 'vSync')
    presentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_OS::memset (&fullScreenDisplayMode, 0, sizeof (struct D3DDISPLAYMODEEX));
    fullScreenDisplayMode.Size = sizeof (struct D3DDISPLAYMODEEX);
    //fullScreenDisplayMode.Width = 0;
    //fullScreenDisplayMode.height = 0;
    //fullScreenDisplayMode.RefreshRate = 0;
    fullScreenDisplayMode.Format = D3DFMT_UNKNOWN;
    fullScreenDisplayMode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

    //usage = (//D3DUSAGE_AUTOGENMIPMAP      |
    //         D3DUSAGE_DEPTHSTENCIL    |
    //         D3DUSAGE_DMAP            |
    //         //D3DUSAGE_DONOTCLIP          |
    //         D3DUSAGE_DYNAMIC         |
    //         D3DUSAGE_NONSECURE       |
    //         //D3DUSAGE_NPATCHES           |
    //         //D3DUSAGE_POINTS             |
    //         D3DUSAGE_RENDERTARGET    |
    //         //D3DUSAGE_RTPATCHES          |
    //         //D3DUSAGE_SOFTWAREPROCESSING |
    //         //D3DUSAGE_TEXTAPI            |
    //         D3DUSAGE_WRITEONLY);//       |
    //         //D3DUSAGE_RESTRICTED_CONTENT |
    //         //D3DUSAGE_RESTRICT_SHARED_RESOURCE |
    //         //D3DUSAGE_RESTRICT_SHARED_RESOURCE_DRIVER);
  }

  UINT                           adapter;
  DWORD                          behaviorFlags; // see also: D3DCREATE
  enum _D3DDEVTYPE               deviceType;
  // *NOTE*: "...If running in full-screen, the focus must be a top level
  //         window. ..."
  HWND                           focusWindow;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*            handle;
#else
  IDirect3DDevice9*              handle;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_SYNCH_MUTEX                lock; // device-
  struct _D3DPRESENT_PARAMETERS_ presentationParameters;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct D3DDISPLAYMODEEX        fullScreenDisplayMode;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  //DWORD                          usage; // see also: D3DUSAGE
  // *IMPORTANT NOTE*: set to the thread that allocates 'handle'
  ACE_thread_t                   threadId;
};

#endif
