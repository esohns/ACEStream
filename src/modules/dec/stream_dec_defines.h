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

#define STREAM_DEC_DECODER_AVI_DEFAULT_NAME_STRING                   "AVI_Decoder"
#if defined (DEEPSPEECH_SUPPORT)
#define STREAM_DEC_DECODER_DEEPSPEECH_DECODER_DEFAULT_NAME_STRING    "DeepSpeech"
#endif // DEEPSPEECH_SUPPORT
#if defined (FAAD_SUPPORT)
#define STREAM_DEC_DECODER_FAAD_DEFAULT_NAME_STRING                  "FAAD"
#endif // FAAD_SUPPORT
#if defined (FESTIVAL_SUPPORT)
#define STREAM_DEC_DECODER_FESTIVAL_DECODER_DEFAULT_NAME_STRING      "Festival"
#endif // FESTIVAL_SUPPORT
#if defined (FLITE_SUPPORT)
#define STREAM_DEC_DECODER_FLITE_DECODER_DEFAULT_NAME_STRING         "Flite"
#endif // FLITE_SUPPORT
#if defined (IMAGEMAGICK_SUPPORT)
#define STREAM_DEC_DECODER_IMAGEMAGICK_DECODER_DEFAULT_NAME_STRING   "ImageMagick_ImageDecoder"
#endif // IMAGEMAGICK_SUPPORT
#if defined (FFMPEG_SUPPORT)
#define STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING   "LibAV_Audio_Decoder"
#define STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING       "LibAV_Converter"
#define STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING         "LibAV_Decoder"
#define STREAM_DEC_DECODER_LIBAV_FILTER_DEFAULT_NAME_STRING          "LibAV_Filter"
#define STREAM_DEC_DECODER_LIBAV_HW_DECODER_DEFAULT_NAME_STRING      "LibAV_HW_Decoder"
#define STREAM_DEC_DECODER_LIBAV_ENCODER_DEFAULT_NAME_STRING         "LibAV_Encoder"
#define STREAM_DEC_DECODER_LIBAV_IMG_DECODER_DEFAULT_NAME_STRING     "LibAV_ImageDecoder"
#define STREAM_DEC_DECODER_LIBAV_SOURCE_DEFAULT_NAME_STRING          "LibAV_Source"
#endif // FFMPEG_SUPPORT
#define STREAM_DEC_DECODER_MPEG_4_DEFAULT_NAME_STRING                "MPEG_4_Decoder"
#define STREAM_DEC_DECODER_MPEG_TS_DEFAULT_NAME_STRING               "MPEG_TS_Decoder"
#define STREAM_DEC_DECODER_MPEG_1LAYER3_DEFAULT_NAME_STRING          "MP3_Decoder"
#define STREAM_DEC_DECODER_M3U_DEFAULT_NAME_STRING                   "M3U_Decoder"
#if defined (OPENCV_SUPPORT)
#define STREAM_DEC_DECODER_OPENCV_QR_DECODER_DEFAULT_NAME_STRING     "OpenCV_QRDecoder"
#endif // OPENCV_SUPPORT
#define STREAM_DEC_DECODER_RGB24_HFLIP_DEFAULT_NAME_STRING           "RGB24HFlip"
#define STREAM_DEC_DECODER_ZIP_DEFAULT_NAME_STRING                   "ZIP_Decoder"

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_DEC_ENCODER_AVI_DEFAULT_NAME_STRING                   "AVI_Encoder"
#else
#if defined (FFMPEG_SUPPORT)
#define STREAM_DEC_ENCODER_AVI_DEFAULT_NAME_STRING                   "AVI_Encoder"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#if defined (SOX_SUPPORT)
#define STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING            "SoX_Effect"
#define STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING         "SoX_Resampler"
#endif // SOX_SUPPORT
#define STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING          "Noise_Source"
#define STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING                   "WAV_Encoder"

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
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_RESIZER_VIDEO              L"Video Resizer DSP DMO"
#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_SPLIT_AVI                  L"AVI Splitter"

#define STREAM_DEC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO               L"Audio Effect"
#endif // ACE_WIN32 || ACE_WIN64

// zlib
#define STREAM_DEC_DEFAULT_ZLIB_WINDOWBITS                           15 // 0,(-)[8-15], see zlib.h
#define STREAM_DEC_ZLIB_WINDOWBITS_GZIP_OFFSET                       16

// noise
#define STREAM_DEC_NOISE_BUFFER_LATENCY_MS                           10 // ms

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

// DeepSpeech STT
// *NOTE*: feed this much sample data between DS_IntermediateDecode calls
#define STREAM_DEC_DEEPSPEECH_DECODE_BUFFER_LENGTH_MS                500 // ms
// *NOTE*: make a new stream after this many decoded words
#define STREAM_DEC_DEEPSPEECH_RESTREAM_WORD_LIMIT                    7 // #words
// *NOTE*: higher values lead to better inference at the cost of computation
#define STREAM_DEC_DEEPSPEECH_DEFAULT_BEAM_WIDTH                     512

// flite TTS
// *NOTE*: feed this much sample data between DS_IntermediateDecode calls
#define STREAM_DEC_FLITE_VOICE_FILENAME_EXTENSION_STRING             ".flitevox"

// MPEG
#define STREAM_DEC_MPEG_TS_PACKET_ID_PAT                             0
#define STREAM_DEC_MPEG_TS_PACKET_ID_NULL                            0x1FFF
#define STREAM_DEC_MPEG_TS_TABLE_ID_PAT                              0
#define STREAM_DEC_MPEG_TS_TABLE_ID_PMT                              2
#define STREAM_DEC_MPEG_TS_PACKET_SIZE                               188 // bytes
#define STREAM_DEC_MPEG_TS_SYNCHRONIZATION_BYTE                      0x47
#define STREAM_DEC_MPEG_TS_STREAM_TYPE_PADDING                       0xBE
#define STREAM_DEC_MPEG_TS_STREAM_TYPE_PRIVATE_2_NAVIGATION_DATA     0xBF

// h264
#define STREAM_DEC_H264_NAL_START_CODE_SIZE                          3 // bytes

// ---------------------------------------

#if defined (FFMPEG_SUPPORT)
// libav/ffmpeg
#define STREAM_DEC_DEFAULT_LIBAV_OUTPUT_SAMPLE_FORMAT                AV_SAMPLE_FMT_S16
#define STREAM_DEC_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT                 AV_PIX_FMT_RGB24
#endif // FFMPEG_SUPPORT

// ---------------------------------------

#if defined (SOX_SUPPORT)
// SoX
#define STREAM_DEC_SOX_BUFFER_SIZE                                   32768 // bytes (default: 8192)
#define STREAM_DEC_SOX_FORMAT_RAW_STRING                             "raw"
#define STREAM_DEC_SOX_FORMAT_WAV_STRING                             "waveaudio"
#define STREAM_DEC_SOX_SAMPLE_BUFFERS                                8192
#endif // SOX_SUPPORT

#endif
