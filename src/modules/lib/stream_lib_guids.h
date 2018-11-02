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

#ifndef STREAM_LIB_GUIDS_H
#define STREAM_LIB_GUIDS_H

#include <Guiddef.h>

// {F9F62434-535B-4934-A695-BE8D10A4C699}
DEFINE_GUID (CLSID_ACEStream_MediaFramework_Source_Filter,
             0xf9f62434,
             0x535b,
             0x4934,
             0xa6, 0x95,
             0xbe, 0x8d, 0x10, 0xa4, 0xc6, 0x99);
// c553f2c0-1529-11d0-b4d1-00805f6cbbea
DEFINE_GUID (CLSID_ACEStream_MediaFramework_Asynch_Source_Filter,
             0xc553f2c0,
             0x1529,
             0x11d0,
             0xb4, 0xd1,
             0x00, 0x80, 0x5f, 0x6c, 0xbb, 0xea);

// {EFE6208A-0A2C-49fa-8A01-3768B559B6DA}
DEFINE_GUID (CLSID_ACEStream_MediaFramework_MF_MediaSource,
             0xefe6208a,
             0xa2c,
             0x49fa,
             0x8a, 0x1,
             0x37, 0x68, 0xb5, 0x59, 0xb6, 0xda);

#endif
