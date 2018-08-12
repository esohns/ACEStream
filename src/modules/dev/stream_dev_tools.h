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

#ifndef STREAM_MODULE_DEV_TOOLS_H
#define STREAM_MODULE_DEV_TOOLS_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <d3d9.h>
#include <d3d9types.h>
#include <dxva2api.h>
#include <guiddef.h>
#include <sdkddkver.h>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include <minwindef.h>
#else
#include <windef.h>
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#include <strmif.h>
#else
#include <linux/videodev2.h>

#include "alsa/asoundlib.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus

#include "stream_dev_common.h"

// forward declarations
class Stream_IAllocator;
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"

class Stream_Module_Device_Tools
{
 public:
  static void initialize (bool = true); // initialize media frameworks ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool getDirect3DDevice (HWND,                            // target window handle
                                 const struct _AMMediaType&,      // media format handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                 IDirect3DDevice9Ex*&,            // return value: Direct3D device handle
#else
                                 IDirect3DDevice9*&,              // return value: Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                 struct _D3DPRESENT_PARAMETERS_&, // return value: Direct3D presentation parameters
                                 IDirect3DDeviceManager9*&,       // return value: Direct3D device manager handle
                                 UINT&);                          // return value: reset token
                                                                  // EnumDisplayMonitors callback data
  struct Stream_EnumDisplayMonitors_CBData
  {
    Stream_EnumDisplayMonitors_CBData ()
     : deviceIdentifier ()
     , handle (NULL)
    {}

    std::string deviceIdentifier;
    HMONITOR    handle;
  };
  static bool getDisplayDevice (const std::string&, // device identifier
                                HMONITOR&);         // return value: monitor handle

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool initializeDirect3DManager (const IDirect3DDevice9Ex*, // Direct3D device handle
#else
  static bool initializeDirect3DManager (const IDirect3DDevice9*,   // Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                         IDirect3DDeviceManager9*&, // return value: Direct3D device manager handle
                                         UINT&);                    // return value: reset token
#else
  static void dump (struct _snd_pcm*); // device handle

  static bool canOverlay (int); // file descriptor
  static bool canStream (int); // file descriptor
  static void dump (int); // file descriptor
  static std::string getALSADeviceName (enum _snd_pcm_stream); // direction
  static bool initializeCapture (int,         // file descriptor
                                 v4l2_memory, // I/O streaming method
                                 __u32&);     // #buffers (in/out)
  static bool initializeOverlay (int,                        // file descriptor
                                 const struct v4l2_window&); // (target) window
  // *IMPORTANT NOTE*: invoke this AFTER VIDIOC_S_FMT, and BEFORE
  //                   VIDIOC_STREAMON
  template <typename MessageType>
  static bool initializeBuffers (int,                               // file descriptor
                                 v4l2_memory,                       // I/O streaming method
                                 __u32,                             // number of buffers
                                 /////////
                                 Stream_Module_Device_BufferMap_t&, // return value: buffer map
                                 /////////
                                 Stream_IAllocator* = NULL);        // allocator
  template <typename MessageType>
  static void finalizeBuffers (int,                                // file descriptor
                               v4l2_memory,                        // I/O streaming method
                               Stream_Module_Device_BufferMap_t&); // buffer map
  static unsigned int queued (int,            // file descriptor
                              unsigned int,   // number of buffers
                              unsigned int&); // return value: #done

  static bool setFormat (struct _snd_pcm*,                                      // device handle
                         const struct Stream_Module_Device_ALSAConfiguration&); // format
  static bool getFormat (struct _snd_pcm*,                                // device handle
                         struct Stream_Module_Device_ALSAConfiguration&); // return value: format
  static bool setFormat (int,                        // device handle file descriptor
                         const struct v4l2_format&); // capture format
  static bool getFormat (int,                  // device handle file descriptor
                         struct v4l2_format&); // return value: format
  // *NOTE*: v4l uses time-per-frame (s) intervals, so the actual frame rate
  //         (fps) is the reciprocal of this value
  static bool getFrameRate (int,                 // device handle file descriptor
                            struct v4l2_fract&); // return value: frame rate (in time-per-frame (s))
  static bool setFrameRate (int,                       // file descriptor
                            const struct v4l2_fract&); // frame rate (in time-per-frame (s))

  static std::string formatToString (__u32); // format (fourcc)
  static std::string formatToString (const struct _snd_pcm_hw_params*); // format

  static struct v4l2_format ffmpegFormatToV4L2Format (enum AVPixelFormat); // format
  static enum AVPixelFormat v4l2FormatToffmpegFormat (__u32); // format (fourcc)
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools (const Stream_Module_Device_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools& operator= (const Stream_Module_Device_Tools&))
};

// include template definitions
#include "stream_dev_tools.inl"

#endif
