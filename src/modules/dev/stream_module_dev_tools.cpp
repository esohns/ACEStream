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
#include "stdafx.h"

#include "stream_module_dev_tools.h"

#include "ace/Log_Msg.h"

#include "strmif.h"

#include "stream_macros.h"

void
Stream_Module_Device_Tools::deleteMediaType (struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::deleteMediaType"));

  // sanity check(s)
  if (!mediaType_inout)
    return;

  if (mediaType_inout->cbFormat)
    CoTaskMemFree ((PVOID)mediaType_inout->pbFormat);
  if (mediaType_inout->pUnk)
    mediaType_inout->pUnk->Release ();
  CoTaskMemFree (mediaType_inout);
  mediaType_inout = NULL;
}
