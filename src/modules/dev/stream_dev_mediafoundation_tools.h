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

#ifndef STREAM_DEVICE_MEDIAFOUNDATION_TOOLS_H
#define STREAM_DEVICE_MEDIAFOUNDATION_TOOLS_H

#include <string>

#include "guiddef.h"
#include "mfidl.h"
#include "mfobjects.h"

#include "ace/Global_Macros.h"

#include "stream_dec_common.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

class Stream_Device_MediaFoundation_Tools
{
 public:
  static void initialize ();

  // -------------------------------------

  // *NOTE*: argument must be MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID or
  //                          MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
  static std::string getDefaultCaptureDevice (REFGUID); // device category
  static Stream_Device_List_t getCaptureDevices (REFGUID); // device category

  //static bool getCaptureFormat (IMFSourceReader*, // source handle
  //                              IMFMediaType*&);  // return value: media type
  //static bool setCaptureFormat (IMFSourceReaderEx*,   // source handle
  //                              const IMFMediaType*); // media type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  static bool getCaptureFormat (IMFMediaSourceEx*, // source handle
#else
  static bool getCaptureFormat (IMFMediaSource*,   // source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                IMFMediaType*&);   // return value: media type
  static bool setCaptureFormat (IMFTopology*,         // topology handle
                                const IMFMediaType*); // media type

  // -------------------------------------

  static bool getMediaSource (const std::string&,  // device identifier
                              REFGUID,             // device category
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                              IMFMediaSourceEx*&); // return value: media source handle
#else
                              IMFMediaSource*&);   // return value: media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

  // -------------------------------------
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  // *NOTE*: if the fourth argument is NULL, the topology has no sink and cannot
  //         be loaded
  static bool loadDeviceTopology (const std::string&,             // device identifier
                                  REFGUID,                        // device category
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                  IMFMediaSourceEx*&,             // input/return value: (capture) media source handle
#else
                                  IMFMediaSource*&,               // input/return value: (capture) media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                  IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
#else
                                  IMFSampleGrabberSinkCallback*,  // sample grabber sink callback handle [NULL: do not use tee/grabber]
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                  IMFTopology*&);                 // return value: topology handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  // -------------------------------------

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_MediaFoundation_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_MediaFoundation_Tools (const Stream_Device_MediaFoundation_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_MediaFoundation_Tools& operator= (const Stream_Device_MediaFoundation_Tools&))

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  static bool setCaptureFormat (IMFMediaSourceEx*,    // source handle
#else
  static bool setCaptureFormat (IMFMediaSource*,      // source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                const IMFMediaType*); // media type
};

#endif
