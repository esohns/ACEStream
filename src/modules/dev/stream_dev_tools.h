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

#include <ace/config-lite.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <map>
#endif
#include <string>

#include <ace/Global_Macros.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <d3d9.h>
#include <d3d9types.h>
#include <dxva2api.h>
#include <guiddef.h>
#include <minwindef.h>
#include <strmif.h>
#include <windef.h>
#else
#include <alsa/asoundlib.h>
#include <linux/videodev2.h>
#endif

#include "stream_dev_exports.h"

class Stream_Dev_Export Stream_Module_Device_Tools
{
 public:
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
  typedef std::map<struct _GUID, std::string, less_guid> GUID2STRING_MAP_T;
  typedef GUID2STRING_MAP_T::const_iterator GUID2STRING_MAP_ITERATOR_T;
  static GUID2STRING_MAP_T Stream_FormatType2StringMap;
#endif

  static void initialize ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool getDirect3DDevice (const HWND,                      // target window handle
                                 const struct _AMMediaType&,      // media format handle
                                 IDirect3DDevice9Ex*&,            // return value: Direct3D device handle
                                 struct _D3DPRESENT_PARAMETERS_&, // return value: Direct3D presentation parameters
                                 IDirect3DDeviceManager9*&,       // return value: Direct3D device manager handle
                                 UINT&);                          // return value: reset token
  static bool initializeDirect3DManager (const IDirect3DDevice9Ex*, // Direct3D device handle
                                         IDirect3DDeviceManager9*&, // return value: Direct3D device manager handle
                                         UINT&);                    // return value: reset token

  static bool isCompressed (REFGUID, // media subtype
                            REFGUID, // device category
                            bool);   // ? media foundation : direct show
  static bool isCompressedAudio (REFGUID, // media subtype
                                 bool);   // ? media foundation : direct show
  static bool isCompressedVideo (REFGUID, // media subtype
                                 bool);   // ? media foundation : direct show

  static bool isChromaLuminance (REFGUID, // media subtype
                                 bool);   // ? media foundation : direct show
  static bool isRGB (REFGUID, // media subtype
                     bool);   // ? media foundation : direct show

  static std::string mediaFormatTypeToString (REFGUID); // GUID
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

  static bool setFormat (struct _snd_pcm*,                               // device handle
                         const Stream_Module_Device_ALSAConfiguration&); // format
  static bool getFormat (struct _snd_pcm*,                         // device handle
                         Stream_Module_Device_ALSAConfiguration&); // return value: format
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
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools (const Stream_Module_Device_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools& operator= (const Stream_Module_Device_Tools&))
};

// include template definitions
#include "stream_dev_tools.inl"

#endif
