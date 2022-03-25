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

#ifndef STREAM_DEV_VFW_TOOLS_H
#define STREAM_DEV_VFW_TOOLS_H

#include "ace/Global_Macros.h"

#include "stream_dev_common.h"

class Stream_Device_VideoForWindows_Tools
{
 public:
  static struct Stream_Device_Identifier getDefaultCaptureDevice (); // return value: device identifier
  static Stream_Device_List_t getCaptureDevices ();
  // *WARNING*: callers must Stream_MediaFramework_DirectShow_Tools::free() the return value !
  static bool getCaptureFormat (const struct Stream_Device_Identifier&, // device identifier
                                struct _AMMediaType&);                  // return value: media type
  static bool hasVideoSourceDialog (HWND); // 'capture window' handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_VideoForWindows_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_VideoForWindows_Tools (const Stream_Device_VideoForWindows_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Device_VideoForWindows_Tools& operator= (const Stream_Device_VideoForWindows_Tools&))
};

#endif
