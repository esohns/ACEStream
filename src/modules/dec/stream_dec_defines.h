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

#ifndef STREAM_DEC_DEFINES_H
#define STREAM_DEC_DEFINES_H

#define STREAM_DEC_DECODER_IMAGEMAGICK_DECODER_DEFAULT_NAME_STRING   "ImageMagick_ImageDecoder"
#define STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING       "LibAV_Converter"
#define STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING         "LibAV_Decoder"
#define STREAM_DEC_DECODER_LIBAV_IMG_DECODER_DEFAULT_NAME_STRING     "LibAV_ImageDecoder"
#define STREAM_DEC_DECODER_MPEG_TS_DEFAULT_NAME_STRING               "MPEG_TSDecoder"
#define STREAM_DEC_DECODER_OPENCV_DECODER_DEFAULT_NAME_STRING        "OpenCV_Decoder"
#define STREAM_DEC_DECODER_ZIP_DEFAULT_NAME_STRING                   "ZIP_Decoder"

#define STREAM_DEC_ENCODER_AVI_DEFAULT_NAME_STRING                   "AVI_Encoder"
#define STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING            "SoX_Effect"
#define STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING                   "WAV_Encoder"

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_PCM                L"WAV Converter"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_RGB                L"Color Space Converter"
//#define STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_YUV     L"AVI Decoder"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_YUV                L"Color Converter DSP DMO"
// *NOTE*: the 'AVI decompressor' (CLSID_AVIDec) supports conversions of YUV
//         to RGB formats via the MSYUV Color Space Converter Codec
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI             L"AVI Decompressor"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264            L"H264 Decompressor"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG            L"MJPG Decompressor"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO               L"Video Renderer"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_RESIZER_VIDEO              L"Video Resizer DSP DMO"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_SPLIT_AVI                  L"AVI Splitter"

#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO               L"Audio Effect"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO               L"Audio Renderer"

#define STREAM_DEC_DIRECTSHOW_FILTER_VIDEO_RENDERER_DEFAULT_FORMAT   MEDIASUBTYPE_RGB32
#endif // ACE_WIN32 || ACE_WIN64

// zlib
#define STREAM_DEC_DEFAULT_ZLIB_WINDOWBITS                           15 // 0,(-)[8-15], see zlib.h
#define STREAM_DEC_ZLIB_WINDOWBITS_GZIP_OFFSET                       16

// stream
#define STREAM_DEC_BUFFER_SIZE                                       16384 // bytes

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
#define STREAM_DEC_DEFAULT_CRUNCH_MESSAGES                           true

// ---------------------------------------

// AVI
#define STREAM_DEC_AVI_JUNK_CHUNK_ALIGN                              2048 // bytes

// MPEG
#define STREAM_DEC_MPEG_TS_PACKET_ID_PAT                             0
#define STREAM_DEC_MPEG_TS_TABLE_ID_PAT                              0
#define STREAM_DEC_MPEG_TS_TABLE_ID_PMT                              2
#define STREAM_DEC_MPEG_TS_PACKET_SIZE                               188 // bytes
#define STREAM_DEC_MPEG_TS_SYNCHRONIZATION_BYTE                      0x47
#define STREAM_DEC_MPEG_TS_STREAM_TYPE_PADDING                       0xBE
#define STREAM_DEC_MPEG_TS_STREAM_TYPE_PRIVATE_2_NAVIGATION_DATA     0xBF

// h264
#define STREAM_DEC_H264_NAL_START_CODE_SIZE                          3 // bytes

// ---------------------------------------

// libav/ffmpeg
#define STREAM_DEC_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT                 AV_PIX_FMT_RGB24

// ---------------------------------------

// SoX
#define STREAM_DEC_SOX_BUFFER_SIZE                                   32768 // bytes (default: 8192)
#define STREAM_DEC_SOX_FORMAT_RAW_STRING                             "raw"
#define STREAM_DEC_SOX_FORMAT_WAV_STRING                             "waveaudio"
#define STREAM_DEC_SOX_SAMPLE_BUFFERS                                8192

// ---------------------------------------

// useful macros
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <cstdint>

#define MAKEFOURCC(a,b,c,d) ((uint32_t)(a << 24)|(uint32_t)(b << 16)|(uint32_t)(c << 8)|(uint32_t)(d))
#endif // ACE_WIN32 || ACE_WIN64

#endif
