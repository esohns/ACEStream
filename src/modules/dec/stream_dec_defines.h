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

#ifndef STREAM_DECODER_DEFINES_H
#define STREAM_DECODER_DEFINES_H

// zlib
#define STREAM_DECODER_DEFAULT_ZLIB_WINDOWBITS     15 // 0,(-)[8-15], see zlib.h
#define STREAM_DECODER_ZLIB_WINDOWBITS_GZIP_OFFSET 16

// parser
// "\0\0"
#define YY_END_OF_BUFFER_CHAR                      0
#define STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE   2

// stream
#define STREAM_DECODER_BUFFER_SIZE                 16384 // bytes

// "crunch" messages (for easier decoding/parsing/processing) ?
// *NOTE*: this comes at the cost of alloc/free, memcopy and locking per
//         (fragmented) message, i.e. should probably be avoided ...
//         OTOH, setting up the buffer correctly allows using the
//         yy_scan_buffer() (instead of yy_scan_bytes()) method, avoiding a copy
//         of the data at that stage --> adding the easier/more robust parsing,
//         this MAY be a viable tradeoff...
// *NOTE*: the current implementation uses both approaches in different phases:
//         - yy_scan_bytes (extra copy) for bisecting the frames
//         - yy_scan_buffer (crunching) during parsing/analysis
// *TODO*: write a (robust) flex-scanner/bison parser that can handle
//         switching of buffers/"backing-up" reliably and stress-test the
//         application to see which option proves to be more efficient...
#define STREAM_DECODER_DEFAULT_CRUNCH_MESSAGES     true
// *IMPORTANT NOTE*: scans buffers in-place (avoids a copy,
//         see: http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html)
//         --> in order to use yy_scan_buffer(), the buffer needs to have been
//             prepared for usage by flex: buffers need two trailing '\0's
//             BEYOND their datas' tail byte (i.e. at positions length() + 1 and
//             length() + 2)
#define STREAM_DECODER_DEFAULT_USE_YY_SCAN_BUFFER  true

// output more debugging information
#define STREAM_DECODER_DEFAULT_LEX_TRACE           false
#define STREAM_DECODER_DEFAULT_YACC_TRACE          false

// ---------------------------------------

#define STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN        2048 // bytes

#define STREAM_DECODER_SOX_WAV_FORMATTYPE_STRING   "waveaudio"
#define STREAM_DECODER_SOX_SAMPLE_BUFFERS          1024

// ---------------------------------------

// useful macros
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mmsyscom.h"
#else
#include <cstdint>

#define MAKEFOURCC(a, b, c, d) ((uint32_t)(a << 24)|(uint32_t)(b << 16)|(uint32_t)(c << 8)|(uint32_t)(d))
#endif

#endif
