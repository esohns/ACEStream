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

#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}
#endif

#include "stream_dec_tools.h"

#include <cmath>

#include <ace/Log_Msg.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <combaseapi.h>
#include <initguid.h> // *NOTE*: this exports DEFINE_GUIDs (see e.g. dxva.h)
#include <dxva.h>
#include <fourcc.h>
#include <guiddef.h>
#include <mfapi.h>
#include <strmif.h>
#include <uuids.h>
#include <wmcodecdsp.h>
#endif

#ifdef __cplusplus
extern "C"
{
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}
#endif

#include "common_tools.h"

#include "stream_macros.h"

// initialize statics
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Stream_Module_Decoder_Tools::GUID2STRING_MAP_T Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap;
Stream_Module_Decoder_Tools::GUID2STRING_MAP_T Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap;
#endif

void
Stream_Module_Decoder_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::initialize"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // DirectShow
  //////////////////////////////////////// AUDIO
  // uncompressed audio
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));

  // MPEG-4 and AAC
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_ADTS_AAC")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR("MPEG_HEAAC")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_LOAS, ACE_TEXT_ALWAYS_CHAR("MPEG_LOAS")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_AAC1, ACE_TEXT_ALWAYS_CHAR("RAW_AAC1")));

  // Dolby
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_DDPLUS, ACE_TEXT_ALWAYS_CHAR("DOLBY_DDPLUS")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVM, ACE_TEXT_ALWAYS_CHAR("DVM")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_SPORT, ACE_TEXT_ALWAYS_CHAR("RAW_SPORT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_SPDIF_TAG_241h, ACE_TEXT_ALWAYS_CHAR("SPDIF_TAG_241h")));

  // miscellaneous
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DRM_Audio, ACE_TEXT_ALWAYS_CHAR("DRM_Audio")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS2, ACE_TEXT_ALWAYS_CHAR("DTS2")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_PCMAudio_Obsolete, ACE_TEXT_ALWAYS_CHAR("PCMAudio_Obsolete")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_RAW_AAC")));

  /////////////////////////////////////// BDA
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_None, ACE_TEXT_ALWAYS_CHAR("None")));

  /////////////////////////////////////// DVD
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_SUBPICTURE, ACE_TEXT_ALWAYS_CHAR("DVD_SUBPICTURE")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_SDDS, ACE_TEXT_ALWAYS_CHAR("SDDS")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_DSI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_DSI")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PCI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PCI")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PROVIDER")));

  /////////////////////////////////////// Line 21
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_BytePair, ACE_TEXT_ALWAYS_CHAR("Line21_BytePair")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_GOPPacket, ACE_TEXT_ALWAYS_CHAR("Line21_GOPPacket")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_VBIRawData, ACE_TEXT_ALWAYS_CHAR("Line21_VBIRawData")));

  /////////////////////////////////////// MPEG-1
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));

  /////////////////////////////////////// MPEG-2
  // MPEG-2 (splitter)
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_VIDEO, ACE_TEXT_ALWAYS_CHAR("MPEG2_VIDEO")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  // MPEG-2 (demultiplexer)
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_PROGRAM, ACE_TEXT_ALWAYS_CHAR("MPEG2_PROGRAM")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT_STRIDE")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ATSC_SI, ACE_TEXT_ALWAYS_CHAR("ATSC_SI")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVB_SI, ACE_TEXT_ALWAYS_CHAR("DVB_SI")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ISDB_SI, ACE_TEXT_ALWAYS_CHAR("ISDB_SI")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2DATA, ACE_TEXT_ALWAYS_CHAR("MPEG2DATA")));
  // MPEG-2 (kernel)

  /////////////////////////////////////// Stream
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AIFF, ACE_TEXT_ALWAYS_CHAR("AIFF")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Asf, ACE_TEXT_ALWAYS_CHAR("Asf")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Avi, ACE_TEXT_ALWAYS_CHAR("Avi")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AU, ACE_TEXT_ALWAYS_CHAR("AU")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DssAudio, ACE_TEXT_ALWAYS_CHAR("DssAudio")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DssVideo, ACE_TEXT_ALWAYS_CHAR("DssVideo")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1SystemStream, ACE_TEXT_ALWAYS_CHAR("MPEG1SystemStream")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WAVE, ACE_TEXT_ALWAYS_CHAR("WAVE")));

  /////////////////////////////////////// VBI
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_RAW8, ACE_TEXT_ALWAYS_CHAR("RAW8")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_TELETEXT, ACE_TEXT_ALWAYS_CHAR("TELETEXT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPS, ACE_TEXT_ALWAYS_CHAR("VPS")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WSS, ACE_TEXT_ALWAYS_CHAR("WSS")));

  /////////////////////////////////////// VIDEO
  // analog video
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_NTSC_M, ACE_TEXT_ALWAYS_CHAR("NTSC_M")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_B, ACE_TEXT_ALWAYS_CHAR("PAL_B")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_D, ACE_TEXT_ALWAYS_CHAR("PAL_D")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_G, ACE_TEXT_ALWAYS_CHAR("PAL_G")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_H, ACE_TEXT_ALWAYS_CHAR("PAL_H")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_I, ACE_TEXT_ALWAYS_CHAR("PAL_I")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_M, ACE_TEXT_ALWAYS_CHAR("PAL_M")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_N, ACE_TEXT_ALWAYS_CHAR("PAL_N")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_B, ACE_TEXT_ALWAYS_CHAR("SECAM_B")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_D, ACE_TEXT_ALWAYS_CHAR("SECAM_D")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_G, ACE_TEXT_ALWAYS_CHAR("SECAM_G")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_H, ACE_TEXT_ALWAYS_CHAR("SECAM_H")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K, ACE_TEXT_ALWAYS_CHAR("SECAM_K")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K1, ACE_TEXT_ALWAYS_CHAR("SECAM_K1")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_L, ACE_TEXT_ALWAYS_CHAR("SECAM_L")));

  // directx video acceleration
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AI44, ACE_TEXT_ALWAYS_CHAR("AI44")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IA44, ACE_TEXT_ALWAYS_CHAR("IA44")));

  // DV video
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvsl, ACE_TEXT_ALWAYS_CHAR("dvsl")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvsd, ACE_TEXT_ALWAYS_CHAR("dvsd")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvhd, ACE_TEXT_ALWAYS_CHAR("dvhd")));

  // H.264
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AVC1, ACE_TEXT_ALWAYS_CHAR("AVC1")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_H264, ACE_TEXT_ALWAYS_CHAR("H264")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_h264, ACE_TEXT_ALWAYS_CHAR("h264")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_X264, ACE_TEXT_ALWAYS_CHAR("X264")));
  //Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_x264, ACE_TEXT_ALWAYS_CHAR("x264")));

  // uncompressed RGB (no alpha)
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB1, ACE_TEXT_ALWAYS_CHAR("RGB1")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB4, ACE_TEXT_ALWAYS_CHAR("RGB4")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB8, ACE_TEXT_ALWAYS_CHAR("RGB8")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB555, ACE_TEXT_ALWAYS_CHAR("RGB555")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB565, ACE_TEXT_ALWAYS_CHAR("RGB565")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB24, ACE_TEXT_ALWAYS_CHAR("RGB24")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32, ACE_TEXT_ALWAYS_CHAR("RGB32")));
  // uncompressed RGB (alpha)
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555, ACE_TEXT_ALWAYS_CHAR("ARGB1555")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32, ACE_TEXT_ALWAYS_CHAR("ARGB32")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444, ACE_TEXT_ALWAYS_CHAR("ARGB4444")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_A2R10G10B10, ACE_TEXT_ALWAYS_CHAR("A2R10G10B10")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_A2B10G10R10, ACE_TEXT_ALWAYS_CHAR("A2B10G10R10")));

  // video mixing renderer (VMR-7)
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX7_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX7_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX7_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX7_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX7_RT")));
  // video mixing renderer (VMR-9)
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX9_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX9_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX9_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX9_RT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX9_RT")));

  // YUV video
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AYUV, ACE_TEXT_ALWAYS_CHAR("AYUV")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YUY2, ACE_TEXT_ALWAYS_CHAR("YUY2")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_UYVY, ACE_TEXT_ALWAYS_CHAR("UYVY")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC1, ACE_TEXT_ALWAYS_CHAR("IMC1")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC2, ACE_TEXT_ALWAYS_CHAR("IMC2")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC3, ACE_TEXT_ALWAYS_CHAR("IMC3")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC4, ACE_TEXT_ALWAYS_CHAR("IMC4")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YV12, ACE_TEXT_ALWAYS_CHAR("YV12")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_NV12, ACE_TEXT_ALWAYS_CHAR("NV12")));
  // other YUV
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_I420, ACE_TEXT_ALWAYS_CHAR("I420")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IF09, ACE_TEXT_ALWAYS_CHAR("IF09")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IYUV, ACE_TEXT_ALWAYS_CHAR("IYUV")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y211, ACE_TEXT_ALWAYS_CHAR("Y211")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y411, ACE_TEXT_ALWAYS_CHAR("Y411")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y41P, ACE_TEXT_ALWAYS_CHAR("Y41P")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YVU9, ACE_TEXT_ALWAYS_CHAR("YVU9")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YVYU, ACE_TEXT_ALWAYS_CHAR("YVYU")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YUYV, ACE_TEXT_ALWAYS_CHAR("YUYV")));

  // miscellaneous
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CFCC, ACE_TEXT_ALWAYS_CHAR("CFCC")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CLJR, ACE_TEXT_ALWAYS_CHAR("CLJR")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CPLA, ACE_TEXT_ALWAYS_CHAR("CPLA")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CLPL, ACE_TEXT_ALWAYS_CHAR("CLPL")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IJPG, ACE_TEXT_ALWAYS_CHAR("IJPG")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MDVF, ACE_TEXT_ALWAYS_CHAR("MDVF")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MJPG, ACE_TEXT_ALWAYS_CHAR("MJPG")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Overlay, ACE_TEXT_ALWAYS_CHAR("Overlay")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Plum, ACE_TEXT_ALWAYS_CHAR("Plum")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTJpeg, ACE_TEXT_ALWAYS_CHAR("QTJpeg")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTMovie, ACE_TEXT_ALWAYS_CHAR("QTMovie")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTRle, ACE_TEXT_ALWAYS_CHAR("QTRle")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTRpza, ACE_TEXT_ALWAYS_CHAR("QTRpza")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTSmc, ACE_TEXT_ALWAYS_CHAR("QTSmc")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_TVMJ, ACE_TEXT_ALWAYS_CHAR("TVMJ")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPVBI, ACE_TEXT_ALWAYS_CHAR("VPVBI")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPVideo, ACE_TEXT_ALWAYS_CHAR("VPVideo")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WAKE, ACE_TEXT_ALWAYS_CHAR("WAKE")));

  ///////////////////////////////////////
  // unknown
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVCS, ACE_TEXT_ALWAYS_CHAR("DVCS")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVSD, ACE_TEXT_ALWAYS_CHAR("DVSD")));

  ///////////////////////////////////////
  // DirectX VA
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeNone, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeNone")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH261_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH261_A")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH261_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH261_B")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH263_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_A")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH263_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_B")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH263_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_C")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH263_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_D")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH263_E, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_E")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH263_F, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_F")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG1_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG1_A")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG1_VLD, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG1_VLD")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG2_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_A")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG2_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_B")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG2_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_C")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG2_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_D")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG2and1_VLD, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2and1_VLD")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_MoComp_NoFGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_MoComp_FGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_IDCT_NoFGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_IDCT_FGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_E, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_NoFGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_F, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_FGT")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_VLD_WithFMOASO_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_WithFMOASO_NoFGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_VLD_Stereo_Progressive_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_Stereo_Progressive_NoFGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_VLD_Stereo_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_Stereo_NoFGT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeH264_VLD_Multiview_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_Multiview_NoFGT")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeWMV8_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV8_PostProc")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeWMV8_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV8_MoComp")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeWMV9_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV9_PostProc")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeWMV9_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV9_MoComp")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeWMV9_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV9_IDCT")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeVC1_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_PostProc")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeVC1_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_MoComp")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeVC1_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_IDCT")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeVC1_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_VLD")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeVC1_D2010, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_D2010")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG4pt2_VLD_Simple, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG4pt2_VLD_Simple")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeHEVC_VLD_Main, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeHEVC_VLD_Main")));
  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_ModeHEVC_VLD_Main10, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeHEVC_VLD_Main10")));

  Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.insert (std::make_pair (DXVA_NoEncrypt, ACE_TEXT_ALWAYS_CHAR ("DXVA_NoEncrypt")));

  //////////////////////////////////////////////////////////////////////////////

  // Media Foundation
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB32, ACE_TEXT_ALWAYS_CHAR ("RGB32")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_ARGB32, ACE_TEXT_ALWAYS_CHAR ("ARGB32")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB24, ACE_TEXT_ALWAYS_CHAR ("RGB24")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB555, ACE_TEXT_ALWAYS_CHAR ("RGB555")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB565, ACE_TEXT_ALWAYS_CHAR ("RGB565")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB8, ACE_TEXT_ALWAYS_CHAR ("RGB8")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_AI44, ACE_TEXT_ALWAYS_CHAR ("AI44")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_AYUV, ACE_TEXT_ALWAYS_CHAR ("AYUV")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YUY2, ACE_TEXT_ALWAYS_CHAR ("YUY2")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YVYU, ACE_TEXT_ALWAYS_CHAR ("YVYU")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YVU9, ACE_TEXT_ALWAYS_CHAR ("YVU9")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_UYVY, ACE_TEXT_ALWAYS_CHAR ("UYVY")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_NV11, ACE_TEXT_ALWAYS_CHAR ("NV11")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_NV12, ACE_TEXT_ALWAYS_CHAR ("NV12")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YV12, ACE_TEXT_ALWAYS_CHAR ("YV12")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_I420, ACE_TEXT_ALWAYS_CHAR ("I420")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_IYUV, ACE_TEXT_ALWAYS_CHAR ("IYUV")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y210, ACE_TEXT_ALWAYS_CHAR ("Y210")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y216, ACE_TEXT_ALWAYS_CHAR ("Y216")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y410, ACE_TEXT_ALWAYS_CHAR ("Y410")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y416, ACE_TEXT_ALWAYS_CHAR ("Y416")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y41P, ACE_TEXT_ALWAYS_CHAR ("Y41P")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y41T, ACE_TEXT_ALWAYS_CHAR ("Y41T")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y42T, ACE_TEXT_ALWAYS_CHAR ("Y42T")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P210, ACE_TEXT_ALWAYS_CHAR ("P210")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P216, ACE_TEXT_ALWAYS_CHAR ("P216")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P010, ACE_TEXT_ALWAYS_CHAR ("P010")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P016, ACE_TEXT_ALWAYS_CHAR ("P016")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v210, ACE_TEXT_ALWAYS_CHAR ("V210")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v216, ACE_TEXT_ALWAYS_CHAR ("V216")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v410, ACE_TEXT_ALWAYS_CHAR ("V410")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP43, ACE_TEXT_ALWAYS_CHAR ("MP43")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP4S, ACE_TEXT_ALWAYS_CHAR ("MP4S")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_M4S2, ACE_TEXT_ALWAYS_CHAR ("M4S2")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP4V, ACE_TEXT_ALWAYS_CHAR ("MP4V")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV1, ACE_TEXT_ALWAYS_CHAR ("WMV1")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV2, ACE_TEXT_ALWAYS_CHAR ("WMV2")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV3, ACE_TEXT_ALWAYS_CHAR ("WMV3")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WVC1, ACE_TEXT_ALWAYS_CHAR ("WVC1")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MSS1, ACE_TEXT_ALWAYS_CHAR ("MSS1")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MSS2, ACE_TEXT_ALWAYS_CHAR ("MSS2")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MPG1, ACE_TEXT_ALWAYS_CHAR ("MPG1")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVSL, ACE_TEXT_ALWAYS_CHAR ("DVSL")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVSD, ACE_TEXT_ALWAYS_CHAR ("DVSD")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVHD, ACE_TEXT_ALWAYS_CHAR ("DVHD")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DV25, ACE_TEXT_ALWAYS_CHAR ("DV25")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DV50, ACE_TEXT_ALWAYS_CHAR ("DV50")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVH1, ACE_TEXT_ALWAYS_CHAR ("DVH1")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVC,  ACE_TEXT_ALWAYS_CHAR ("DVC")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H264, ACE_TEXT_ALWAYS_CHAR ("H264")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MJPG, ACE_TEXT_ALWAYS_CHAR ("MJPG")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_420O, ACE_TEXT_ALWAYS_CHAR ("420O")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_HEVC, ACE_TEXT_ALWAYS_CHAR ("HEVC")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_HEVC_ES, ACE_TEXT_ALWAYS_CHAR ("HEVC_ES")));
#if (WINVER >= _WIN32_WINNT_WIN8)
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H263, ACE_TEXT_ALWAYS_CHAR ("H263")));
#endif
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H264_ES, ACE_TEXT_ALWAYS_CHAR ("H264_ES")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MPEG2, ACE_TEXT_ALWAYS_CHAR ("MPEG2")));

  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Float, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_DTS, ACE_TEXT_ALWAYS_CHAR ("DTS")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3_SPDIF")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_DRM, ACE_TEXT_ALWAYS_CHAR ("DRM")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudioV8, ACE_TEXT_ALWAYS_CHAR ("WMAudioV8")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudioV9, ACE_TEXT_ALWAYS_CHAR ("WMAudioV9")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudio_Lossless, ACE_TEXT_ALWAYS_CHAR ("WMAudio_Lossless")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMASPDIF, ACE_TEXT_ALWAYS_CHAR ("WMASPDIF")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MSP1, ACE_TEXT_ALWAYS_CHAR ("MSP1")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MP3, ACE_TEXT_ALWAYS_CHAR ("MP3")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MPEG, ACE_TEXT_ALWAYS_CHAR ("MPEG")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AAC, ACE_TEXT_ALWAYS_CHAR ("AAC")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_ADTS, ACE_TEXT_ALWAYS_CHAR ("ADTS")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_NB, ACE_TEXT_ALWAYS_CHAR ("AMR_NB")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("AMR_WB")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_WP, ACE_TEXT_ALWAYS_CHAR ("AMR_WP")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3")));
  Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_DDPlus, ACE_TEXT_ALWAYS_CHAR ("Dolby_DDPlus")));
#endif
}

bool
Stream_Module_Decoder_Tools::isCompressedVideo (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isCompressedVideo"));

  return (!Stream_Module_Decoder_Tools::isRGB (format_in) &&
          !Stream_Module_Decoder_Tools::isChromaLuminance (format_in));
}

bool
Stream_Module_Decoder_Tools::isChromaLuminance (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isChromaLuminance"));

  switch (format_in)
  {
    case AV_PIX_FMT_YUV420P:   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    case AV_PIX_FMT_YUYV422:   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    case AV_PIX_FMT_YUV422P:   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    case AV_PIX_FMT_YUV444P:   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    case AV_PIX_FMT_YUV410P:   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    case AV_PIX_FMT_YUV411P:   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    case AV_PIX_FMT_YUVJ420P:  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV420P and setting color_range
    case AV_PIX_FMT_YUVJ422P:  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
    case AV_PIX_FMT_YUVJ444P:  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV444P and setting color_range
    case AV_PIX_FMT_UYVY422:   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    case AV_PIX_FMT_UYYVYY411: ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    case AV_PIX_FMT_NV12:      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    case AV_PIX_FMT_NV21:      ///< as above, but U and V bytes are swapped
    case AV_PIX_FMT_YUV440P:   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    case AV_PIX_FMT_YUVJ440P:  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV440P and setting color_range
    case AV_PIX_FMT_YUVA420P:  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    case AV_PIX_FMT_YUV420P16LE:  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV420P16BE:  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV422P16LE:  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV422P16BE:  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P16LE:  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P16BE:  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV420P9BE: ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P9LE: ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV420P10BE:///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P10LE:///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV422P10BE:///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P10LE:///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P9BE: ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P9LE: ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P10BE:///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P10LE:///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV422P9BE: ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P9LE: ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUVA422P:  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    case AV_PIX_FMT_YUVA444P:  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    case AV_PIX_FMT_YUVA420P9BE:  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    case AV_PIX_FMT_YUVA420P9LE:  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    case AV_PIX_FMT_YUVA422P9BE:  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    case AV_PIX_FMT_YUVA422P9LE:  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    case AV_PIX_FMT_YUVA444P9BE:  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    case AV_PIX_FMT_YUVA444P9LE:  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    case AV_PIX_FMT_YUVA420P10BE: ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA420P10LE: ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA422P10BE: ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA422P10LE: ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA444P10BE: ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA444P10LE: ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA420P16BE: ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA420P16LE: ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA422P16BE: ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA422P16LE: ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA444P16BE: ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA444P16LE: ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    case AV_PIX_FMT_NV16:         ///< interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    case AV_PIX_FMT_NV20LE:       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_NV20BE:       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YVYU422:   ///< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb
    case AV_PIX_FMT_YUV420P12BE: ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P12LE: ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV420P14BE: ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P14LE: ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV422P12BE: ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P12LE: ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV422P14BE: ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P14LE: ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P12BE: ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P12LE: ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P14BE: ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P14LE: ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUVJ411P:    ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV411P and setting color_range
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case AV_PIX_FMT_YUV440P10LE: ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    case AV_PIX_FMT_YUV440P10BE: ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    case AV_PIX_FMT_YUV440P12LE: ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    case AV_PIX_FMT_YUV440P12BE: ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    case AV_PIX_FMT_AYUV64LE:    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    case AV_PIX_FMT_AYUV64BE:    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    case AV_PIX_FMT_P010LE: ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
    case AV_PIX_FMT_P010BE: ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian
    case AV_PIX_FMT_P016LE: ///< like NV12, with 16bpp per component, little-endian
    case AV_PIX_FMT_P016BE: ///< like NV12, with 16bpp per component, big-endian
#endif
      return true;
    default: break;
  } // end SWITCH

  return false;
}
bool
Stream_Module_Decoder_Tools::isRGB (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isRGB"));

  switch (format_in)
  {
    case AV_PIX_FMT_RGB24:     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    case AV_PIX_FMT_BGR24:     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    case AV_PIX_FMT_BGR8:      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    case AV_PIX_FMT_BGR4:      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    case AV_PIX_FMT_BGR4_BYTE: ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    case AV_PIX_FMT_RGB8:      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    case AV_PIX_FMT_RGB4:      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    case AV_PIX_FMT_RGB4_BYTE: ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    case AV_PIX_FMT_ARGB:      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    case AV_PIX_FMT_RGBA:      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    case AV_PIX_FMT_ABGR:      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    case AV_PIX_FMT_BGRA:      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
    case AV_PIX_FMT_RGB48BE:   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    case AV_PIX_FMT_RGB48LE:   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian
    case AV_PIX_FMT_RGB565BE:  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    case AV_PIX_FMT_RGB565LE:  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    case AV_PIX_FMT_RGB555BE:  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
    case AV_PIX_FMT_RGB555LE:  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_BGR565BE:  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    case AV_PIX_FMT_BGR565LE:  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    case AV_PIX_FMT_BGR555BE:  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
    case AV_PIX_FMT_BGR555LE:  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_RGB444LE:  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_RGB444BE:  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
    case AV_PIX_FMT_BGR444LE:  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_BGR444BE:  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
    case AV_PIX_FMT_BGR48BE:   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    case AV_PIX_FMT_BGR48LE:   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian
    case AV_PIX_FMT_RGBA64BE:     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    case AV_PIX_FMT_RGBA64LE:     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    case AV_PIX_FMT_BGRA64BE:     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    case AV_PIX_FMT_BGRA64LE:     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    case AV_PIX_FMT_0RGB:        ///< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
    case AV_PIX_FMT_RGB0:        ///< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
    case AV_PIX_FMT_0BGR:        ///< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
    case AV_PIX_FMT_BGR0:        ///< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined
      return true;
    default: break;
  } // end SWITCH

  return false;
}

std::string
Stream_Module_Decoder_Tools::errorToString (int error_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::errorToString"));

  std::string result;

  int result_2 = -1;
  char buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));

  result_2 = av_strerror (error_in,
                          buffer,
                          sizeof (buffer));
  if (result_2 < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to av_strerror(%d): \"%m\", continuing\n"),
                error_in));

  result = buffer;

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
std::string
Stream_Module_Decoder_Tools::GUIDToString (REFGUID GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::GUIDToString"));

  std::string result;

  OLECHAR GUID_string[CHARS_IN_GUID];
  ACE_OS::memset (GUID_string, 0, sizeof (GUID_string));
  int result_2 = StringFromGUID2 (GUID_in,
                                  GUID_string, CHARS_IN_GUID);
  ACE_ASSERT (result_2 == CHARS_IN_GUID);

#if defined (OLE2ANSI)
  result = GUID_string;
#else
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (GUID_string));
#endif

  return result;
}
struct _GUID
Stream_Module_Decoder_Tools::StringToGUID (const std::string& string_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::StringToGUID"));

  struct _GUID result = GUID_NULL;

  HRESULT result_2 = E_FAIL;
#if defined (OLE2ANSI)
  result_2 = CLSIDFromString (string_in.c_str (), &result);
#else
  result_2 =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (string_in.c_str ()), &result);
#endif
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (string_in.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return GUID_NULL;
  } // end IF

  return result;
}

bool
Stream_Module_Decoder_Tools::isRGB (REFGUID subType_in,
                                    bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isRGB"));

  if (useMediaFoundation_in)
    return ((subType_in == MFVideoFormat_RGB32)  ||
            (subType_in == MFVideoFormat_ARGB32) ||
            (subType_in == MFVideoFormat_RGB24)  ||
            (subType_in == MFVideoFormat_RGB555) ||
            (subType_in == MFVideoFormat_RGB565) ||
            (subType_in == MFVideoFormat_RGB8));

  return (// uncompressed RGB (no alpha)
          (subType_in == MEDIASUBTYPE_RGB1)   ||
          (subType_in == MEDIASUBTYPE_RGB4)   ||
          (subType_in == MEDIASUBTYPE_RGB8)   ||
          (subType_in == MEDIASUBTYPE_RGB555) ||
          (subType_in == MEDIASUBTYPE_RGB565) ||
          (subType_in == MEDIASUBTYPE_RGB24)  ||
          (subType_in == MEDIASUBTYPE_RGB32)  ||
          // uncompressed RGB (alpha)
          (subType_in == MEDIASUBTYPE_ARGB1555)    ||
          (subType_in == MEDIASUBTYPE_ARGB32)      ||
          (subType_in == MEDIASUBTYPE_ARGB4444)    ||
          (subType_in == MEDIASUBTYPE_A2R10G10B10) ||
          (subType_in == MEDIASUBTYPE_A2B10G10R10) ||
          // video mixing renderer (VMR-7)
          (subType_in == MEDIASUBTYPE_RGB32_D3D_DX7_RT)    ||
          (subType_in == MEDIASUBTYPE_RGB16_D3D_DX7_RT)    ||
          (subType_in == MEDIASUBTYPE_ARGB32_D3D_DX7_RT)   ||
          (subType_in == MEDIASUBTYPE_ARGB4444_D3D_DX7_RT) ||
          (subType_in == MEDIASUBTYPE_ARGB1555_D3D_DX7_RT) ||
          // video mixing renderer (VMR-9)
          (subType_in == MEDIASUBTYPE_RGB32_D3D_DX9_RT)    ||
          (subType_in == MEDIASUBTYPE_RGB16_D3D_DX9_RT)    ||
          (subType_in == MEDIASUBTYPE_ARGB32_D3D_DX9_RT)   ||
          (subType_in == MEDIASUBTYPE_ARGB4444_D3D_DX9_RT) ||
          (subType_in == MEDIASUBTYPE_ARGB1555_D3D_DX9_RT));
}
bool
Stream_Module_Decoder_Tools::isChromaLuminance (REFGUID subType_in,
                                                bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isChromaLuminance"));

  if (useMediaFoundation_in)
    return ((subType_in == MFVideoFormat_AYUV) ||
            (subType_in == MFVideoFormat_YUY2) ||
            (subType_in == MFVideoFormat_YVYU) ||
            (subType_in == MFVideoFormat_YVU9) ||
            (subType_in == MFVideoFormat_UYVY) ||
            (subType_in == MFVideoFormat_NV11) ||
            (subType_in == MFVideoFormat_NV12) ||
            (subType_in == MFVideoFormat_YV12) ||
            (subType_in == MFVideoFormat_I420) ||
            (subType_in == MFVideoFormat_IYUV) ||
            (subType_in == MFVideoFormat_Y210) ||
            (subType_in == MFVideoFormat_Y216) ||
            (subType_in == MFVideoFormat_Y410) ||
            (subType_in == MFVideoFormat_Y416) ||
            (subType_in == MFVideoFormat_Y41P) ||
            (subType_in == MFVideoFormat_Y41T) ||
            (subType_in == MFVideoFormat_Y42T) ||
            (subType_in == MFVideoFormat_P210) ||
            (subType_in == MFVideoFormat_P216) ||
            (subType_in == MFVideoFormat_P010) ||
            (subType_in == MFVideoFormat_P016) ||
            (subType_in == MFVideoFormat_v210) ||
            (subType_in == MFVideoFormat_v216) ||
            (subType_in == MFVideoFormat_v410));

  return ((subType_in == MEDIASUBTYPE_AYUV) ||
          (subType_in == MEDIASUBTYPE_YUY2) ||
          (subType_in == MEDIASUBTYPE_UYVY) ||
          (subType_in == MEDIASUBTYPE_IMC1) ||
          (subType_in == MEDIASUBTYPE_IMC2) ||
          (subType_in == MEDIASUBTYPE_IMC3) ||
          (subType_in == MEDIASUBTYPE_IMC4) ||
          (subType_in == MEDIASUBTYPE_YV12) ||
          (subType_in == MEDIASUBTYPE_NV12) ||
          //
          (subType_in == MEDIASUBTYPE_I420) ||
          (subType_in == MEDIASUBTYPE_IF09) ||
          (subType_in == MEDIASUBTYPE_IYUV) ||
          (subType_in == MEDIASUBTYPE_Y211) ||
          (subType_in == MEDIASUBTYPE_Y411) ||
          (subType_in == MEDIASUBTYPE_Y41P) ||
          (subType_in == MEDIASUBTYPE_YVU9) ||
          (subType_in == MEDIASUBTYPE_YVYU) ||
          (subType_in == MEDIASUBTYPE_YUYV));
}

enum AVCodecID
Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVCodecID (REFGUID mediaSubType_in,
                                                          bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVCodecID"));

  enum AVCodecID result = AV_CODEC_ID_NONE;

  // sanity check(s)
  if (Stream_Module_Decoder_Tools::isRGB (mediaSubType_in, useMediaFoundation_in) ||
      Stream_Module_Decoder_Tools::isChromaLuminance (mediaSubType_in, useMediaFoundation_in))
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("media type subtype is neither RGB nor Chroma/Luminance (was: \"%s\"), aborting\n"),
    //            ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaSubType_in, useMediaFoundation_in).c_str ())));
    return result;
  } // end IF

  if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_MJPG))
    result = AV_CODEC_ID_MJPEG;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media type subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaSubType_in, useMediaFoundation_in).c_str ())));
  } // end ELSE

  return result;
}

enum AVPixelFormat
Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (REFGUID mediaSubType_in,
                                                              bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat"));

  enum AVPixelFormat result = AV_PIX_FMT_NONE;

  // sanity check(s)
  if (!Stream_Module_Decoder_Tools::isRGB (mediaSubType_in, useMediaFoundation_in) &&
      !Stream_Module_Decoder_Tools::isChromaLuminance (mediaSubType_in, useMediaFoundation_in))
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("media type subtype is neither RGB nor Chroma/Luminance (was: \"%s\"), aborting\n"),
    //            ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaSubType_in, useMediaFoundation_in).c_str ())));
    return result;
  } // end IF

  if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB1))
    result = AV_PIX_FMT_MONOBLACK;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB4))
    result = AV_PIX_FMT_RGB4;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB8))
    result = AV_PIX_FMT_RGB8;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB555))
    result = AV_PIX_FMT_RGB555;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB565))
    result = AV_PIX_FMT_RGB565;
  // *IMPORTANT NOTE*: MEDIASUBTYPE_RGB24 actually has a 'BGR24' memory layout
  //                   see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd407253(v=vs.85).aspx
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB24))
    result = AV_PIX_FMT_BGR24;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32))
    result = AV_PIX_FMT_RGB32;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32))
    result = AV_PIX_FMT_ARGB;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444))
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2R10G10B10))
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2B10G10R10))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32_D3D_DX7_RT))
    result = AV_PIX_FMT_RGB32;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB16_D3D_DX7_RT))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32_D3D_DX7_RT))
    result = AV_PIX_FMT_ARGB;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444_D3D_DX7_RT))
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555_D3D_DX7_RT))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32_D3D_DX9_RT))
    result = AV_PIX_FMT_RGB32;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB16_D3D_DX9_RT))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32_D3D_DX9_RT))
    result = AV_PIX_FMT_ARGB;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444_D3D_DX9_RT))
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555_D3D_DX9_RT))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_AYUV))
    result = AV_PIX_FMT_YUVA444P;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YUY2))
    result = AV_PIX_FMT_YUYV422;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_UYVY))
    result = AV_PIX_FMT_UYVY422;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC1))
    result = AV_PIX_FMT_P016;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC2))
    result = AV_PIX_FMT_NV12;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC3))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC4))
    result = AV_PIX_FMT_NV21;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YV12))
    result = AV_PIX_FMT_YUV420P;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_NV12))
    result = AV_PIX_FMT_NV12;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_I420))
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IF09))
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IYUV))
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_Y211))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_Y411))
    result = AV_PIX_FMT_UYYVYY411;
  //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YVU9))
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YVYU))
    result = AV_PIX_FMT_YVYU422;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YUYV))
    result = AV_PIX_FMT_YUYV422;
  else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YUYV))
    result = AV_PIX_FMT_YUYV422;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media type subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaSubType_in, useMediaFoundation_in).c_str ())));
  } // end ELSE

  return result;
}

std::string
Stream_Module_Decoder_Tools::mediaSubTypeToString (REFGUID GUID_in,
                                                   bool useMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::mediaSubTypeToString"));

  std::string result;

  // within FOURCC range ? --> use helper class
  if ((GUID_in.Data2 == 0x0000) &&
      (GUID_in.Data3 == 0x0010) &&
      ((GUID_in.Data4[0] == 0x80) &&
       (GUID_in.Data4[1] == 0x00) &&
       (GUID_in.Data4[2] == 0x00) &&
       (GUID_in.Data4[3] == 0xAA) &&
       (GUID_in.Data4[4] == 0x00) &&
       (GUID_in.Data4[5] == 0x38) &&
       (GUID_in.Data4[6] == 0x9B) &&
       (GUID_in.Data4[7] == 0x71)))
  {
    FOURCCMap fourcc_map (&GUID_in);

    return Stream_Module_Decoder_Tools::FOURCCToString (fourcc_map.GetFOURCC ());
  } // end IF

  GUID2STRING_MAP_ITERATOR_T iterator;
  if (useMediaFoundation_in)
  {
    iterator =
      Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.find (GUID_in);
    if (iterator == Stream_Module_Decoder_Tools::Stream_MediaFoundationMediaSubType2StringMap.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (GUID_in).c_str ())));
      return result;
    } // end IF
  } // end IF
  else
  {
    iterator =
      Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.find (GUID_in);
    if (iterator == Stream_Module_Decoder_Tools::Stream_DirectShowMediaSubType2StringMap.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (GUID_in).c_str ())));
      return result;
    } // end IF
  } // end ELSE

  return (*iterator).second;
}
#else
enum AVCodecID
Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecID (enum AVPixelFormat pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecID"));

  enum AVCodecID result = AV_CODEC_ID_NONE;

  // sanity check(s)
  ACE_ASSERT (!Stream_Module_Decoder_Tools::isRGB (pixelFormat_in) &&
              !Stream_Module_Decoder_Tools::isChromaLuminance (pixelFormat_in));

  switch (pixelFormat_in)
  { // *TODO*: find a better way to do this
    case AV_PIX_FMT_YUVJ422P:
      result = AV_CODEC_ID_MJPEG; break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %d), aborting\n"),
                  pixelFormat_in));
      break;
    }
  } // end SWITCH

  return result;
}
#endif

std::string
Stream_Module_Decoder_Tools::compressionFormatToString (enum Stream_Decoder_CompressionFormatType format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::compressionFormatToString"));

  std::string result = ACE_TEXT_ALWAYS_CHAR ("Invalid");

  switch (format_in)
  {
    case STREAM_COMPRESSION_FORMAT_NONE:
      result = ACE_TEXT_ALWAYS_CHAR ("None"); break;
    case STREAM_COMPRESSION_FORMAT_GZIP:
      result = ACE_TEXT_ALWAYS_CHAR ("GZIP"); break;
    case STREAM_COMPRESSION_FORMAT_ZLIB:
      result = ACE_TEXT_ALWAYS_CHAR ("ZLIB"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return result;
}

void
Stream_Module_Decoder_Tools::sinus (double frequency_in,
                                    unsigned int sampleRate_in,
                                    unsigned int sampleSize_in, // 'data' sample
                                    unsigned int channels_in,
                                    char* buffer_in,
                                    unsigned int samplesToWrite_in, // #'data' samples
                                    double& phase_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::sinus"));

  static double maximum_phase = 2.0 * M_PI;
  double step =
    (maximum_phase * frequency_in) / static_cast<double> (sampleRate_in);
  unsigned int bytes_per_sample = sampleSize_in / channels_in;
  unsigned int maximum_value = (1 << ((bytes_per_sample * 8) - 1)) - 1;
  double phase = phase_inout;
  int value = 0;
  char* pointer_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value = static_cast<int> (sin (phase) * maximum_value);
    for (unsigned int j = 0; j < channels_in; ++j)
    {
      for (unsigned int k = 0; k < bytes_per_sample; ++k)
      {
        if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
          *(pointer_p + k) = (value >> (k * 8)) & 0xFF;
        else
          *(pointer_p + bytes_per_sample - 1 - k) = (value >> (k * 8)) & 0xFF;
      } // end FOR
      pointer_p += bytes_per_sample;
    } // end FOR
    phase += step;
    if (phase >= maximum_phase) phase -= maximum_phase;
  } // end FOR
  phase_inout = phase;
}

bool
Stream_Module_Decoder_Tools::convert (struct SwsContext* context_in,
                                      unsigned int sourceWidth_in,
                                      unsigned int sourceHeight_in,
                                      enum AVPixelFormat sourcePixelFormat_in,
                                      uint8_t* sourceBuffers_in[],
                                      unsigned int targetWidth_in,
                                      unsigned int targetHeight_in,
                                      enum AVPixelFormat targetPixelFormat_in,
                                      uint8_t* targetBuffers_in[])
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::convert"));

  // sanity check(s)
  ACE_ASSERT (sourcePixelFormat_in != targetPixelFormat_in);

  int flags = (SWS_FAST_BILINEAR | // interpolation
               SWS_POINT);
  struct SwsContext* context_p =
      (context_in ? context_in
                  : sws_getCachedContext (NULL,
                                          sourceWidth_in, sourceHeight_in, sourcePixelFormat_in,
                                          targetWidth_in, targetHeight_in, targetPixelFormat_in,
                                          flags,                             // flags
                                          NULL, NULL,
                                          0));                               // parameters
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
    return false;
  } // end IF

  bool result = Stream_Module_Decoder_Tools::scale (context_p,
                                                    sourceWidth_in,
                                                    sourceHeight_in,
                                                    sourcePixelFormat_in,
                                                    sourceBuffers_in,
                                                    targetWidth_in,
                                                    targetHeight_in,
                                                    targetPixelFormat_in,
                                                    targetBuffers_in);

  // clean up
  if (!context_in)
    sws_freeContext (context_p);

  return result;
}

bool
Stream_Module_Decoder_Tools::scale (struct SwsContext* context_in,
                                    unsigned int sourceWidth_in,
                                    unsigned int sourceHeight_in,
                                    enum AVPixelFormat sourcePixelFormat_in,
                                    uint8_t* sourceBuffers_in[],
                                    unsigned int targetWidth_in,
                                    unsigned int targetHeight_in,
                                    enum AVPixelFormat targetPixelFormat_in,
                                    uint8_t* targetBuffers_in[])
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::scale"));

  struct SwsContext* context_p =
      (context_in ? context_in
                  : sws_getCachedContext (NULL,
                                          sourceWidth_in, sourceHeight_in, sourcePixelFormat_in,
                                          targetWidth_in, targetHeight_in, targetPixelFormat_in,
                                          0,                                 // flags
                                          NULL, NULL,
                                          0));                               // parameters
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
    return false;
  } // end IF

  int result = -1;
  int in_linesize[4] = { 0, 0, 0, 0 };
  int out_linesize[4] = { 0, 0, 0, 0 };
  result = av_image_fill_linesizes (in_linesize,
                                    sourcePixelFormat_in,
                                    static_cast<int> (sourceWidth_in));
  ACE_ASSERT (result != -1);
  result = av_image_fill_linesizes (out_linesize,
                                    targetPixelFormat_in,
                                    static_cast<int> (targetWidth_in));
  ACE_ASSERT (result != -1);

  sws_scale (context_p,
             sourceBuffers_in, in_linesize,
             0, sourceHeight_in,
             targetBuffers_in, out_linesize);

  // clean up
  if (!context_in)
    sws_freeContext (context_p);

  return true;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
void
Stream_Module_Decoder_Tools::ALSA2SOX (const Stream_Module_Device_ALSAConfiguration& format_in,
                                       struct sox_encodinginfo_t& encoding_out,
                                       struct sox_signalinfo_t& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::ALSA2SOX"));

//  int result = -1;

  // initialize return value(s)
  ACE_OS::memset (&encoding_out, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&format_out, 0, sizeof (struct sox_signalinfo_t));

  encoding_out.encoding = SOX_ENCODING_SIGN2;
//  enum _snd_pcm_format ALSA_format = SND_PCM_FORMAT_UNKNOWN;
//  unsigned int channels = 0;
//  unsigned int sample_rate = 0;
//  int subunit_direction = 0;

//  result = snd_pcm_hw_params_get_format (format_in,
//                                         &ALSA_format);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", returning\n"),
//                ACE_TEXT (snd_strerror (result))));
//    return;
//  } // end IF
//  switch (ALSA_format)
  switch (format_in.format)
  {
    // PCM 'formats'
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S16_BE:
      break;
    case SND_PCM_FORMAT_U16_LE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U16_BE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_S8:
      break;
    case SND_PCM_FORMAT_U8:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_MU_LAW:
      encoding_out.encoding = SOX_ENCODING_ULAW;
      break;
    case SND_PCM_FORMAT_A_LAW:
      encoding_out.encoding = SOX_ENCODING_ALAW;
      break;
    case SND_PCM_FORMAT_S32_LE:
      break;
    case SND_PCM_FORMAT_S32_BE:
      break;
    case SND_PCM_FORMAT_U32_LE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U32_BE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_S24_LE:
      break;
    case SND_PCM_FORMAT_S24_BE:
      break;
    case SND_PCM_FORMAT_U24_LE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U24_BE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_FLOAT_LE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT_BE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT64_LE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT64_BE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ALSA audio frame format (was: %d), returning\n"),
//                  ALSA_format));
                  format_in.format));
      return;
    }
  } // end SWITCH

//  result = snd_pcm_hw_params_get_channels (format_in,
//                                           &channels);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", returning\n"),
//                ACE_TEXT (snd_strerror (result))));
//    return;
//  } // end IF
//  result = snd_pcm_hw_params_get_rate (format_in,
//                                       &sample_rate, &subunit_direction);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate(): \"%s\", returning\n"),
//                ACE_TEXT (snd_strerror (result))));
//    return;
//  } // end IF

//      encoding_out.compression = 0.0;
//  encoding_out.bits_per_sample = snd_pcm_format_width (ALSA_format);
  encoding_out.bits_per_sample = snd_pcm_format_width (format_in.format);
  encoding_out.reverse_bytes = sox_option_default;
  encoding_out.reverse_nibbles = sox_option_default;
  encoding_out.reverse_bits = sox_option_default;
  encoding_out.opposite_endian = sox_false;

//  format_out.rate = sample_rate;
//  format_out.channels = channels;
//  format_out.precision = snd_pcm_format_width (ALSA_format);
    format_out.rate = format_in.rate;
    format_out.channels = format_in.channels;
    format_out.precision = snd_pcm_format_width (format_in.format);
//      format_out.length = 0;
//      format_out.mult = NULL;
}

#endif
