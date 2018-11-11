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

#ifndef STREAM_VIS_TOOLS_H
#define STREAM_VIS_TOOLS_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>

#if defined (__cplusplus)
extern "C"
{
#include "libavformat/avformat.h"
}
#endif /* __cplusplus */
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"

#include "stream_vis_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_defines.h"
#endif // ACE_WIN32 || ACE_WIN64

class Stream_Visualization_Tools
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool initialize (enum Stream_Visualization_Framework = STREAM_VIS_FRAMEWORK_DEFAULT,
                          bool = true); // initialize COM ?
  static void finalize (enum Stream_Visualization_Framework = STREAM_VIS_FRAMEWORK_DEFAULT,
                        bool = true); // finalize COM ?
#endif // ACE_WIN32 || ACE_WIN64

  static std::string rendererToModuleName (enum Stream_Visualization_AudioRenderer);
  static std::string rendererToModuleName (enum Stream_Visualization_VideoRenderer);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static enum AVPixelFormat mediaSubTypeToAVPixelFormat (REFGUID); // media foundation subtype
#endif // ACE_WIN32 || ACE_WIN64
 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_Tools (const Stream_Visualization_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_Tools& operator= (const Stream_Visualization_Tools&))
};

#endif
