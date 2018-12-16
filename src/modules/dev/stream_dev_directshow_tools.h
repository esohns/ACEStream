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

#ifndef STREAM_DEVICE_DIRECTSHOW_TOOLS_H
#define STREAM_DEVICE_DIRECTSHOW_TOOLS_H

#include <string>

#include <winnt.h>
#include <Guiddef.h>
#include <Ks.h>
#include <strmif.h>

#include "ace/Global_Macros.h"

#include "common_ui_common.h"

#include "stream_lib_common.h"
#include "stream_lib_directshow_common.h"

#include "stream_dev_common.h"

class Stream_Device_DirectShow_Tools
{
 public:
  static bool initialize (bool = true); // initialize COM ?
  static void finalize (bool = true); // finalize COM ?

  // device
  // *NOTE*: returns the devices' "FriendlyName"
  static std::string devicePathToString (const std::string&); // device path
  static std::string devicePath (const std::string&); // device 'friendly' name

  // *NOTE*: returns the devices' 'path'
  static std::string getDefaultCaptureDevice (REFGUID); // (capture) device category
  static Stream_Device_List_t getCaptureDevices (REFGUID); // (capture) device category

  // -------------------------------------

  // format
  static bool isMediaTypeBottomUp (const struct _AMMediaType&);
  static Common_Identifiers_t getCaptureSubFormats (IAMStreamConfig*);
  static Common_UI_Resolutions_t getCaptureResolutions (IAMStreamConfig*,
                                                        REFGUID = GUID_NULL); // media subtype {GUID_NULL: all}
  static Common_UI_Framerates_t getCaptureFramerates (IAMStreamConfig*,
                                                      REFGUID,                        // media subtype
                                                      const Common_UI_Resolution_t&); // resolution
  // *IMPORTANT NOTE*: caller must deleteMediaType() the return value !
  static bool getCaptureFormat (IGraphBuilder*,        // graph builder handle
                                REFGUID,               // device category
                                struct _AMMediaType&); // return value: media type
  static bool getVideoCaptureFormat (IGraphBuilder*,        // graph builder handle
                                     REFGUID,               // media subtype {GUID_NULL: default}
                                     LONG,                  // width {0: any}
                                     LONG,                  // height {0: any}
                                     unsigned int,          // framerate {0: any}
                                     struct _AMMediaType&); // return value: media type
  static void listCaptureFormats (IBaseFilter*,         // filter handle
                                  REFGUID = GUID_NULL); // format type {GUID_NULL: all}
  static bool setCaptureFormat (IGraphBuilder*,              // graph builder handle
                                REFGUID,                     // device category
                                const struct _AMMediaType&); // media type

  // -------------------------------------

  // filter graph
  // *NOTE*: loads the (capture device) filter and puts it into an empty graph
  static bool loadDeviceGraph (const std::string&,                         // device path
                               REFGUID,                                    // device category
                               IGraphBuilder*&,                            // return value: (capture) graph handle
                               IAMBufferNegotiation*&,                     // return value: capture filter output pin buffer allocator configuration handle
                               IAMStreamConfig*&,                          // return value: format configuration handle
                               Stream_MediaFramework_DirectShow_Graph_t&); // return value: graph layout

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_DirectShow_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_DirectShow_Tools (const Stream_Device_DirectShow_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_DirectShow_Tools& operator= (const Stream_Device_DirectShow_Tools&))
};

#endif
