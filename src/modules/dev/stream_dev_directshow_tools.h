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

#ifndef STREAM_MODULE_DEV_DIRECTSHOW_TOOLS_H
#define STREAM_MODULE_DEV_DIRECTSHOW_TOOLS_H

#include <string>

#include <winnt.h>
#include <guiddef.h>
//#include <coguid.h>
#include <CGuid.h>
#include <strmif.h>

#include "ace/Global_Macros.h"

#include "stream_lib_common.h"

class Stream_Module_Device_DirectShow_Tools
{
 public:
  static bool initialize (bool = true); // initialize COM ?
  static void finalize (bool = true); // finalize COM ?

  // device
  // *NOTE*: returns the devices' "FriendlyName"
  static std::string devicePathToString (const std::string&); // device path
  // *NOTE*: returns the devices' 'path'
  static std::string getDefaultDevice (REFGUID); // (capture) device category

  // -------------------------------------

  // *IMPORTANT NOTE*: caller must deleteMediaType() the return value !
  static bool getCaptureFormat (IGraphBuilder*,         // graph builder handle
                                REFGUID,                // device category
                                struct _AMMediaType*&); // return value: media type
  static bool getVideoCaptureFormat (IGraphBuilder*,         // graph builder handle
                                     REFGUID,                // media subtype
                                     LONG,                   // width {0: any}
                                     LONG,                   // height {0: any}
                                     struct _AMMediaType*&); // return value: media type
  static void listCaptureFormats (IBaseFilter*,         // filter handle
                                  REFGUID = GUID_NULL); // format type {GUID_NULL: all}
  static bool setCaptureFormat (IGraphBuilder*,              // graph builder handle
                                REFGUID,                     // device category
                                const struct _AMMediaType&); // media type

  // -------------------------------------

  // *NOTE*: loads the (capture device) filter and puts it into an empty graph
  static bool loadDeviceGraph (const std::string&,                         // device path
                               REFGUID,                                    // device category
                               IGraphBuilder*&,                            // return value: (capture) graph handle
                               IAMBufferNegotiation*&,                     // return value: capture filter output pin buffer allocator configuration handle
                               IAMStreamConfig*&,                          // return value: format configuration handle
                               Stream_MediaFramework_DirectShow_Graph_t&); // return value: graph layout

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_DirectShow_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_DirectShow_Tools (const Stream_Module_Device_DirectShow_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_DirectShow_Tools& operator= (const Stream_Module_Device_DirectShow_Tools&))
};

#endif
