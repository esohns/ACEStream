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

#ifndef STREAM_LIB_PIPEWIRE_TOOLS_H
#define STREAM_LIB_PIPEWIRE_TOOLS_H

#include "pipewire/pipewire.h"

#include "ace/Global_Macros.h"

#include "stream_lib_alsa_common.h"

class Stream_MediaFramework_Pipewire_Tools
{
 public:
  static void freeProxy (struct pw_loop*,   // loop handle
                         struct pw_proxy*); // node proxy handle
  static void finalizeStream (struct pw_loop*,    // loop handle
                              struct pw_stream*); // stream handle

  static bool getVolumeControl (struct Stream_MediaFramework_ALSA_MediaType&, // format
                                struct pw_thread_loop*&,                      // (return value:) loop handle
                                struct pw_context*&,                          // return value: context handle
                                struct pw_core*&,                             // return value: core handle
                                struct pw_stream*&);                          // return value: stream handle
  static void freeVolumeControl (struct pw_thread_loop*&, // loop handle
                                 struct pw_context*&,     // context handle
                                 struct pw_core*&,        // core handle
                                 struct pw_stream*&);     // stream handle

  static bool setVolumeLevel (struct pw_stream*, // stream handle
                              uint32_t,          // #channels
                              float);            // level [0.0f-1.0f]
  static bool setVolumeLevel (struct pw_loop*,   // loop handle
                              // struct pw_proxy*, // node (!) proxy handle
                              struct pw_stream*, // stream handle
                              uint32_t,          // #channels
                              float);            // level [0.0f-1.0f]

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Pipewire_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Pipewire_Tools (const Stream_MediaFramework_Pipewire_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Pipewire_Tools& operator= (const Stream_MediaFramework_Pipewire_Tools&))
};

#endif
