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

#include "ace/config-lite.h"

#define MODULE_MISC_DUMP_DEFAULT_NAME_STRING                        "Dump"
#define MODULE_MISC_PARSER_DEFAULT_NAME_STRING                      "YaccParser"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// DirectShow
#define MODULE_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS                  30 // ==> max. #frames(/sec)

#define MODULE_MISC_DS_WIN32_FILTER_SOURCE_FRAME_INTERVAL           20 // ms
// *NOTE*: if the graph (i.e. usually the renderers'-) (default) allocator
//         supplies the sample buffers (instead of the (source) filter), and the
//         stream message type does not implement IMediaSample, the 'push'
//         strategy (FillBuffer/Receive) involves a(n additional) memcpy of
//         (inbound) frame data
// *NOTE*: the 'pull' strategy is implemented via IAsynchReader (Request/
//         WaitForNext)
#define MODULE_MISC_DS_WIN32_FILTER_SOURCE_DEFAULT_PUSH             false

#define MODULE_MISC_DS_WIN32_ALLOCATOR_NAME                         "ACEStream DirectShow Allocator"
#define MODULE_MISC_DS_WIN32_FILTER_NAME_ASYNCH_SOURCE              "ACEStream DirectShow Asynch Source"
#define MODULE_MISC_DS_WIN32_FILTER_NAME_ASYNCH_SOURCE_L            L"ACEStream DirectShow Asynch Source"
#define MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE                     "ACEStream DirectShow Source"
#define MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L                   L"ACEStream DirectShow Source"
#define MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME                 L"Output"

// MediaFoundation
// *NOTE*: #samples each stream tries to hold in its queue
#define MODULE_MISC_MF_WIN32_MEDIA_SOURCE_SAMPLE_QUEUE_SIZE         2;

#define MODULE_MISC_MF_WIN32_BYTESTREAMHANDLER_DESCRIPTION          "ACEStream Source ByteStreamHandler"
#define MODULE_MISC_MF_WIN32_REG_BYTESTREAMHANDLERS_KEY             "Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers"
#endif

#define STREAM_MISC_PARSER_DEFAULT_LEX_TRACE                        false
#define STREAM_MISC_PARSER_DEFAULT_YACC_TRACE                       false

#define YY_END_OF_BUFFER_CHAR                                       0 // "\0\0"
#define STREAM_MISC_PARSER_FLEX_BUFFER_BOUNDARY_SIZE                2
// *IMPORTANT NOTE*: scans buffers in-place (avoids a copy,
//         see: http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html)
//         --> in order to use yy_scan_buffer(), the buffer needs to have been
//             prepared for usage by flex: buffers need two trailing '\0's
//             BEYOND their datas' tail byte (i.e. at positions length() + 1 and
//             length() + 2)
#define STREAN_MISC_PARSER_FLEX_USE_YY_SCAN_BUFFER                  true

#endif
