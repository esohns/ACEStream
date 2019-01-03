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

#ifndef STREAM_DEVICE_TOOLS_H
#define STREAM_DEVICE_TOOLS_H

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
#include "alsa/asoundlib.h"

#include "linux/videodev2.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "common_ui_common.h"

#include "stream_dev_common.h"

#include "stream_lib_common.h"
#include "stream_lib_alsa_common.h"
#include "stream_lib_v4l_common.h"

// forward declarations
class Stream_IAllocator;
#endif // ACE_WIN32 || ACE_WIN64

class Stream_Device_Tools
{
 public:
  static void initialize (bool = true); // initialize media frameworks ?

  static std::string getDefaultAudioCaptureDevice ();
  static std::string getDefaultVideoCaptureDevice ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // ALSA
  static bool setFormat (struct _snd_pcm*,                                    // device handle
                         const struct Stream_MediaFramework_ALSA_MediaType&); // media type
  static bool getFormat (struct _snd_pcm*,                              // device handle
                         struct Stream_MediaFramework_ALSA_MediaType&); // return value: media type

  static std::string getDeviceName (enum _snd_pcm_stream); // direction
  static std::string formatToString (const struct _snd_pcm_hw_params*); // format

  static void dump (struct _snd_pcm*); // device handle

  // v4l
  static Stream_Device_List_t getVideoCaptureDevices ();
  static struct v4l2_pix_format getVideoCaptureFormat (int,                           // file descriptor
                                                       __u32,                         // pixel format
                                                       const Common_UI_Resolution_t&, // resolution {0: any}
                                                       const struct v4l2_fract&);     // framerate {0/1: any}
  static Stream_MediaFramework_V4L_CaptureFormats_t getCaptureSubFormats (int); // file descriptor
  static Common_UI_Resolutions_t getCaptureResolutions (int,    // file descriptor
                                                        __u32); // pixel format
  static Common_UI_Framerates_t getCaptureFramerates (int,                            // file descriptor
                                                      __u32,                          // pixel format
                                                      const Common_UI_Resolution_t&); // resolution

  static struct Stream_MediaFramework_V4L_MediaType defaultCaptureFormat (const std::string&); // device identifier

  static void dump (int); // file descriptor

  static bool canOverlay (int); // file descriptor
  static bool canStream (int); // file descriptor
  static bool initializeCapture (int,         // file descriptor
                                 v4l2_memory, // I/O streaming method
                                 __u32&);     // #buffers (in/out)
  static bool initializeOverlay (int,                        // file descriptor
                                 const struct v4l2_window&); // (target) window
  // *IMPORTANT NOTE*: invoke this AFTER VIDIOC_S_FMT, and BEFORE
  //                   VIDIOC_STREAMON
  template <typename MessageType>
  static bool initializeBuffers (int,                        // file descriptor
                                 v4l2_memory,                // I/O streaming method
                                 __u32,                      // number of buffers
                                 /////////
                                 Stream_Device_BufferMap_t&, // return value: buffer map
                                 /////////
                                 Stream_IAllocator* = NULL); // allocator
  template <typename MessageType>
  static void finalizeBuffers (int,                         // file descriptor
                               v4l2_memory,                 // I/O streaming method
                               Stream_Device_BufferMap_t&); // buffer map
  static unsigned int queued (int,            // file descriptor
                              unsigned int,   // number of buffers
                              unsigned int&); // return value: #done

  static bool setFormat (int,                            // device handle file descriptor
                         const struct v4l2_pix_format&); // capture format
  static bool getFormat (int,                  // device handle file descriptor
                         struct v4l2_format&); // return value: format
  // *NOTE*: v4l uses time-per-frame (s) intervals, so the actual frame rate
  //         (fps) is the reciprocal of this value
  static bool getFrameRate (int,                 // device handle file descriptor
                            struct v4l2_fract&); // return value: frame rate (in time-per-frame (s))
  static bool setFrameRate (int,                       // file descriptor
                            const struct v4l2_fract&); // frame rate (in time-per-frame (s))

  inline static std::string formatToString (__u32 format_in) { std::string result; result += ((char)(format_in & 0x000000FF)); result += ((char)((format_in >> 8) & 0x000000FF)); result += ((char)((format_in >> 16) & 0x000000FF)); result += ((char)((format_in >> 24) & 0x000000FF)); return result; }
  static std::string formatToString (int,    // file descriptor
                                     __u32); // format (fourcc)

  static __u32 ffmpegFormatToV4L2Format (enum AVPixelFormat); // format
  static enum AVPixelFormat v4l2FormatToffmpegFormat (__u32); // format (fourcc)
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_Tools (const Stream_Device_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_Tools& operator= (const Stream_Device_Tools&))
};

// include template definitions
#include "stream_dev_tools.inl"

#endif
