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

#ifndef STREAM_MODULE_VIS_TOOLS_H
#define STREAM_MODULE_VIS_TOOLS_H

#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
}
#endif /* __cplusplus */

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#endif

#include "ace/Global_Macros.h"

#include "stream_vis_exports.h"

class Stream_Vis_Export Stream_Module_Visualization_Tools
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   static enum AVPixelFormat mediaSubType2AVPixelFormat (REFGUID); // media foundation subtype
#endif
 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Visualization_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Visualization_Tools (const Stream_Module_Visualization_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Visualization_Tools& operator= (const Stream_Module_Visualization_Tools&))
};

#endif
