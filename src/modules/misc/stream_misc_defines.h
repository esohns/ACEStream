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

#ifndef STREAM_MODULE_MISC_DEFINES_H
#define STREAM_MODULE_MISC_DEFINES_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS        60 // ==> max. #frames(/sec)
#define MODULE_MISC_DS_WIN32_FILTER_SOURCE_FRAME_INTERVAL 20 // ms
// *NOTE*: if the graph (i.e. usually the renderers'-) default allocator
//         supplies the sample buffers (instead of the (source) filter), and the
//         stream message type does not implement IMediaSample, the 'pull'
//         strategy involves a(n additional) memcpy of (inbound) frame data
#define MODULE_MISC_DS_WIN32_FILTER_SOURCE_DEFAULT_PUSH   true

#define MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE           "ACEStream DirectShow Source"
#define MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L         L"ACEStream DirectShow Source"
#define MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME       L"Output"
#endif

#endif
