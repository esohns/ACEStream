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

#include "stream_vis_tools.h"

#include "ace/Log_Msg.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dshow.h"
 //#include "ksuuids.h"
#include "strmif.h"
#include "wmcodecdsp.h"
#endif

#include "common_tools.h"

#include "stream_macros.h"

#include "stream_vis_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// initialize statics
Stream_Module_Visualization_Tools::GUID2STRING_MAP_T Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap;
#endif

void
Stream_Module_Visualization_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::initialize"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  /////////////////////////////////////// AUDIO
  // uncompressed audio
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));

  // MPEG-4 and AAC
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_ADTS_AAC")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR("MPEG_HEAAC")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_LOAS, ACE_TEXT_ALWAYS_CHAR("MPEG_LOAS")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_AAC1, ACE_TEXT_ALWAYS_CHAR("RAW_AAC1")));

  // Dolby
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_DDPLUS, ACE_TEXT_ALWAYS_CHAR("DOLBY_DDPLUS")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVM, ACE_TEXT_ALWAYS_CHAR("DVM")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_SPORT, ACE_TEXT_ALWAYS_CHAR("RAW_SPORT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_SPDIF_TAG_241h, ACE_TEXT_ALWAYS_CHAR("SPDIF_TAG_241h")));

  // miscellaneous
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DRM_Audio, ACE_TEXT_ALWAYS_CHAR("DRM_Audio")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS2, ACE_TEXT_ALWAYS_CHAR("DTS2")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_PCMAudio_Obsolete, ACE_TEXT_ALWAYS_CHAR("PCMAudio_Obsolete")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_RAW_AAC")));

  /////////////////////////////////////// BDA
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_None, ACE_TEXT_ALWAYS_CHAR("None")));

  /////////////////////////////////////// DVD
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_SUBPICTURE, ACE_TEXT_ALWAYS_CHAR("DVD_SUBPICTURE")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_SDDS, ACE_TEXT_ALWAYS_CHAR("SDDS")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_DSI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_DSI")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PCI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PCI")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PROVIDER")));

  /////////////////////////////////////// Line 21
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_BytePair, ACE_TEXT_ALWAYS_CHAR("Line21_BytePair")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_GOPPacket, ACE_TEXT_ALWAYS_CHAR("Line21_GOPPacket")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_VBIRawData, ACE_TEXT_ALWAYS_CHAR("Line21_VBIRawData")));

  /////////////////////////////////////// MPEG-1
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));

  /////////////////////////////////////// MPEG-2
  // MPEG-2 (splitter)
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_VIDEO, ACE_TEXT_ALWAYS_CHAR("MPEG2_VIDEO")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  // MPEG-2 (demultiplexer)
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_PROGRAM, ACE_TEXT_ALWAYS_CHAR("MPEG2_PROGRAM")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT_STRIDE")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ATSC_SI, ACE_TEXT_ALWAYS_CHAR("ATSC_SI")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVB_SI, ACE_TEXT_ALWAYS_CHAR("DVB_SI")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ISDB_SI, ACE_TEXT_ALWAYS_CHAR("ISDB_SI")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2DATA, ACE_TEXT_ALWAYS_CHAR("MPEG2DATA")));
  // MPEG-2 (kernel)

  /////////////////////////////////////// Stream
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AIFF, ACE_TEXT_ALWAYS_CHAR("AIFF")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Asf, ACE_TEXT_ALWAYS_CHAR("Asf")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Avi, ACE_TEXT_ALWAYS_CHAR("Avi")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AU, ACE_TEXT_ALWAYS_CHAR("AU")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DssAudio, ACE_TEXT_ALWAYS_CHAR("DssAudio")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DssVideo, ACE_TEXT_ALWAYS_CHAR("DssVideo")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1SystemStream, ACE_TEXT_ALWAYS_CHAR("MPEG1SystemStream")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WAVE, ACE_TEXT_ALWAYS_CHAR("WAVE")));

  /////////////////////////////////////// VBI
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_RAW8, ACE_TEXT_ALWAYS_CHAR("RAW8")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_TELETEXT, ACE_TEXT_ALWAYS_CHAR("TELETEXT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPS, ACE_TEXT_ALWAYS_CHAR("VPS")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WSS, ACE_TEXT_ALWAYS_CHAR("WSS")));

  /////////////////////////////////////// VIDEO
  // analog video
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_NTSC_M, ACE_TEXT_ALWAYS_CHAR("NTSC_M")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_B, ACE_TEXT_ALWAYS_CHAR("PAL_B")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_D, ACE_TEXT_ALWAYS_CHAR("PAL_D")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_G, ACE_TEXT_ALWAYS_CHAR("PAL_G")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_H, ACE_TEXT_ALWAYS_CHAR("PAL_H")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_I, ACE_TEXT_ALWAYS_CHAR("PAL_I")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_M, ACE_TEXT_ALWAYS_CHAR("PAL_M")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_N, ACE_TEXT_ALWAYS_CHAR("PAL_N")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_B, ACE_TEXT_ALWAYS_CHAR("SECAM_B")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_D, ACE_TEXT_ALWAYS_CHAR("SECAM_D")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_G, ACE_TEXT_ALWAYS_CHAR("SECAM_G")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_H, ACE_TEXT_ALWAYS_CHAR("SECAM_H")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K, ACE_TEXT_ALWAYS_CHAR("SECAM_K")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K1, ACE_TEXT_ALWAYS_CHAR("SECAM_K1")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_L, ACE_TEXT_ALWAYS_CHAR("SECAM_L")));

  // directx video acceleration
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AI44, ACE_TEXT_ALWAYS_CHAR("AI44")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IA44, ACE_TEXT_ALWAYS_CHAR("IA44")));

  // DV video
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvsl, ACE_TEXT_ALWAYS_CHAR("dvsl")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvsd, ACE_TEXT_ALWAYS_CHAR("dvsd")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvhd, ACE_TEXT_ALWAYS_CHAR("dvhd")));

  // H.264
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AVC1, ACE_TEXT_ALWAYS_CHAR("AVC1")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_H264, ACE_TEXT_ALWAYS_CHAR("H264")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_h264, ACE_TEXT_ALWAYS_CHAR("h264")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_X264, ACE_TEXT_ALWAYS_CHAR("X264")));
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_x264, ACE_TEXT_ALWAYS_CHAR("x264")));

  // uncompressed RGB (no alpha)
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB1, ACE_TEXT_ALWAYS_CHAR("RGB1")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB4, ACE_TEXT_ALWAYS_CHAR("RGB4")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB8, ACE_TEXT_ALWAYS_CHAR("RGB8")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB555, ACE_TEXT_ALWAYS_CHAR("RGB555")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB565, ACE_TEXT_ALWAYS_CHAR("RGB565")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB24, ACE_TEXT_ALWAYS_CHAR("RGB24")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32, ACE_TEXT_ALWAYS_CHAR("RGB32")));
  // uncompressed RGB (alpha)
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555, ACE_TEXT_ALWAYS_CHAR("ARGB1555")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32, ACE_TEXT_ALWAYS_CHAR("ARGB32")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444, ACE_TEXT_ALWAYS_CHAR("ARGB4444")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_A2R10G10B10, ACE_TEXT_ALWAYS_CHAR("A2R10G10B10")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_A2B10G10R10, ACE_TEXT_ALWAYS_CHAR("A2B10G10R10")));

  // video mixing renderer (VMR-7)
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX7_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX7_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX7_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX7_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX7_RT")));
  // video mixing renderer (VMR-9)
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX9_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX9_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX9_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX9_RT")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX9_RT")));

  // YUV video
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AYUV, ACE_TEXT_ALWAYS_CHAR("AYUV")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YUY2, ACE_TEXT_ALWAYS_CHAR("YUY2")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_UYVY, ACE_TEXT_ALWAYS_CHAR("UYVY")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC1, ACE_TEXT_ALWAYS_CHAR("IMC1")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC2, ACE_TEXT_ALWAYS_CHAR("IMC2")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC3, ACE_TEXT_ALWAYS_CHAR("IMC3")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC4, ACE_TEXT_ALWAYS_CHAR("IMC4")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YV12, ACE_TEXT_ALWAYS_CHAR("YV12")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_NV12, ACE_TEXT_ALWAYS_CHAR("NV12")));
  // other YUV
  //Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_I420, ACE_TEXT_ALWAYS_CHAR("I420")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IF09, ACE_TEXT_ALWAYS_CHAR("IF09")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IYUV, ACE_TEXT_ALWAYS_CHAR("IYUV")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y211, ACE_TEXT_ALWAYS_CHAR("Y211")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y411, ACE_TEXT_ALWAYS_CHAR("Y411")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y41P, ACE_TEXT_ALWAYS_CHAR("Y41P")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YVU9, ACE_TEXT_ALWAYS_CHAR("YVU9")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YVYU, ACE_TEXT_ALWAYS_CHAR("YVYU")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YUYV, ACE_TEXT_ALWAYS_CHAR("YUYV")));

  // miscellaneous
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CFCC, ACE_TEXT_ALWAYS_CHAR("CFCC")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CLJR, ACE_TEXT_ALWAYS_CHAR("CLJR")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CPLA, ACE_TEXT_ALWAYS_CHAR("CPLA")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CLPL, ACE_TEXT_ALWAYS_CHAR("CLPL")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IJPG, ACE_TEXT_ALWAYS_CHAR("IJPG")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MDVF, ACE_TEXT_ALWAYS_CHAR("MDVF")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MJPG, ACE_TEXT_ALWAYS_CHAR("MJPG")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Overlay, ACE_TEXT_ALWAYS_CHAR("Overlay")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Plum, ACE_TEXT_ALWAYS_CHAR("Plum")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTJpeg, ACE_TEXT_ALWAYS_CHAR("QTJpeg")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTMovie, ACE_TEXT_ALWAYS_CHAR("QTMovie")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTRle, ACE_TEXT_ALWAYS_CHAR("QTRle")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTRpza, ACE_TEXT_ALWAYS_CHAR("QTRpza")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTSmc, ACE_TEXT_ALWAYS_CHAR("QTSmc")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_TVMJ, ACE_TEXT_ALWAYS_CHAR("TVMJ")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPVBI, ACE_TEXT_ALWAYS_CHAR("VPVBI")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPVideo, ACE_TEXT_ALWAYS_CHAR("VPVideo")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WAKE, ACE_TEXT_ALWAYS_CHAR("WAKE")));

  ///////////////////////////////////////
  // unknown
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVCS, ACE_TEXT_ALWAYS_CHAR("DVCS")));
  Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVSD, ACE_TEXT_ALWAYS_CHAR("DVSD")));
#endif
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_Module_Visualization_Tools::loadDeviceGraph (const std::string& deviceName_in,
                                             ICaptureGraphBuilder2*& ICaptureGraphBuilder2_inout,
                                             IAMStreamConfig*& IAMStreamConfig_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::loadDeviceGraph"));

  // sanity check(s)
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release ();
    IAMStreamConfig_out = NULL;
  } // end IF

  HRESULT result = E_FAIL;
  IGraphBuilder* builder_p = NULL;
  IBaseFilter* filter_p = NULL;
  if (!ICaptureGraphBuilder2_inout)
  {
    result =
      CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL,
                        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2,
                        (void**)&ICaptureGraphBuilder2_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_CaptureGraphBuilder2): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (ICaptureGraphBuilder2_inout);

    result = CoCreateInstance (CLSID_FilterGraph, NULL,
                               CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
                               (void**)&builder_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      ICaptureGraphBuilder2_inout->Release ();
      ICaptureGraphBuilder2_inout = NULL;

      return false;
    } // end IF

    result = ICaptureGraphBuilder2_inout->SetFiltergraph (builder_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ICaptureGraphBuilder2::SetFiltergraph(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      builder_p->Release ();
      ICaptureGraphBuilder2_inout->Release ();
      ICaptureGraphBuilder2_inout = NULL;

      return false;
    } // end IF
  } // end IF
  else
  {
    result = ICaptureGraphBuilder2_inout->GetFiltergraph (&builder_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF

    IEnumFilters* enumerator_p = NULL;
    result = builder_p->EnumFilters (&enumerator_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      builder_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (enumerator_p);

    while (S_OK == enumerator_p->Next (1, &filter_p, NULL))
    {
      ACE_ASSERT (filter_p);
      result = builder_p->RemoveFilter (filter_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGrapBuilder::RemoveFilter(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        filter_p->Release ();
        enumerator_p->Release ();
        builder_p->Release ();

        return false;
      } // end IF
      enumerator_p->Reset ();

      filter_p->Release ();
      filter_p = NULL;
    } // end WHILE
    enumerator_p->Release ();
  } // end ELSE
  ACE_ASSERT (builder_p);

  ICreateDevEnum* enumerator_p = NULL;
  result =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);

  IEnumMoniker* enum_moniker_p = NULL;
  result =
    enumerator_p->CreateClassEnumerator (CLSID_VideoInputDeviceCategory,
                                         &enum_moniker_p,
                                         0);
  if (result != S_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(CLSID_VideoInputDeviceCategory): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    enumerator_p->Release ();
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    return false;
  } // end IF
  ACE_ASSERT (enum_moniker_p);
  enumerator_p->Release ();

  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  VARIANT variant;
  while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
  {
    ACE_ASSERT (moniker_p);

    properties_p = NULL;
    result = moniker_p->BindToStorage (0, 0, IID_PPV_ARGS (&properties_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      enum_moniker_p->Release ();
      moniker_p->Release ();
      builder_p->Release ();
      ICaptureGraphBuilder2_inout->Release ();
      ICaptureGraphBuilder2_inout = NULL;

      return false;
    } // end IF
    ACE_ASSERT (properties_p);

    VariantInit (&variant);
    result = properties_p->Read (L"FriendlyName", &variant, 0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyBag::Read(FriendlyName): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      enum_moniker_p->Release ();
      moniker_p->Release ();
      properties_p->Release ();
      builder_p->Release ();
      ICaptureGraphBuilder2_inout->Release ();
      ICaptureGraphBuilder2_inout = NULL;

      return false;
    } // end IF
    properties_p->Release ();
    ACE_Wide_To_Ascii converter (variant.bstrVal);
    VariantClear (&variant);

    if (deviceName_in.empty () ||
        (ACE_OS::strcmp (deviceName_in.c_str (),
                         converter.char_rep ()) == 0))
      break;

    moniker_p->Release ();
    moniker_p = NULL;
  } // end WHILE
  enum_moniker_p->Release ();
  if (!moniker_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("device (was: \"%s\") not found, aborting\n"),
                ACE_TEXT (deviceName_in.c_str ())));

    // clean up
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF

  result = moniker_p->BindToObject (0, 0, IID_IBaseFilter,
                                    (void**)&filter_p);
  moniker_p->Release ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMoniker::BindToObject(IID_IBaseFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  result = builder_p->AddFilter (filter_p,
                                 MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF

  IEnumPins* enumerator_2 = NULL;
  result = filter_p->EnumPins (&enumerator_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (enumerator_2);
  filter_p->Release ();

  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  GUID GUID_i;
  ACE_OS::memset (&GUID_i, 0, sizeof (GUID));
  DWORD returned_size = 0;
  while (S_OK == enumerator_2->Next (1, &pin_p, NULL))
  {
    ACE_ASSERT (pin_p);

    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_2->Release ();
      builder_p->Release ();
      ICaptureGraphBuilder2_inout->Release ();
      ICaptureGraphBuilder2_inout = NULL;

      return false;
    } // end IF
    if (pin_direction != PINDIR_OUTPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF
    property_set_p = NULL;
    result = pin_p->QueryInterface (IID_PPV_ARGS (&property_set_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryInterface(IKsPropertySet): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();
      builder_p->Release ();
      ICaptureGraphBuilder2_inout->Release ();
      ICaptureGraphBuilder2_inout = NULL;

      return false;
    } // end IF
    ACE_ASSERT (property_set_p);
    result = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                                  NULL, 0,
                                  &GUID_i, sizeof (GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      property_set_p->Release ();
      pin_p->Release ();
      enumerator_p->Release ();
      builder_p->Release ();
      ICaptureGraphBuilder2_inout->Release ();
      ICaptureGraphBuilder2_inout = NULL;

      return false;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (GUID));
    if (GUID_i == PIN_CATEGORY_CAPTURE)
      break;

    property_set_p->Release ();
    pin_p->Release ();
    pin_p = NULL;
  } // end WHILE
  enumerator_2->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("0x%@: no capture pin found, aborting\n"),
                filter_p));

    // clean up
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF

  result = pin_p->QueryInterface (IID_IAMStreamConfig,
                                  (void**)&IAMStreamConfig_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (IAMStreamConfig_out);
  pin_p->Release ();
  builder_p->Release ();

  return true;
}
bool
Stream_Module_Visualization_Tools::assembleGraph (ICaptureGraphBuilder2* builder_in,
                                           const std::list<std::wstring>& graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::assembleGraph"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!graph_in.empty ());

  IGraphBuilder* builder_p = NULL;
  HRESULT result = builder_in->GetFiltergraph (&builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (builder_p);

  IBaseFilter* filter_p = NULL;
  std::list<std::wstring>::const_iterator iterator = graph_in.begin ();
  result =
    builder_p->FindFilterByName ((*iterator).c_str (),
                                 &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    builder_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  IEnumPins* enumerator_p = NULL;
  result = filter_p->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    builder_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  while (enumerator_p->Next (1, &pin_p, NULL) == S_OK)
  {
    ACE_ASSERT (pin_p);

    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    if (pin_direction != PINDIR_OUTPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF

    break;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));

    // clean up
    builder_p->Release ();

    return false;
  } // end IF
  IPin* pin_2 = NULL;
  //struct _PinInfo pin_info;
  //ACE_OS::memset (&pin_info, 0, sizeof (struct _PinInfo));

  for (++iterator;
       iterator != graph_in.end ();
       ++iterator)
  {
    filter_p = NULL;
    result =
      builder_p->FindFilterByName ((*iterator).c_str (),
                                   &filter_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (filter_p);

    result = filter_p->EnumPins (&enumerator_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      pin_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (enumerator_p);
    filter_p->Release ();
    while (enumerator_p->Next (1, &pin_2, NULL) == S_OK)
    {
      ACE_ASSERT (pin_2);

      //result = pin_2->QueryPinInfo (&pin_info);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IPin::QueryPinInfo(): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      //  // clean up
      //  pin_2->Release ();
      //  enumerator_p->Release ();
      //  pin_p->Release ();
      //  builder_p->Release ();

      //  return false;
      //} // end IF

      result = pin_2->QueryDirection (&pin_direction);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        pin_2->Release ();
        enumerator_p->Release ();
        pin_p->Release ();
        builder_p->Release ();

        return false;
      } // end IF
      if (pin_direction != PINDIR_INPUT)
      {
        pin_2->Release ();
        pin_2 = NULL;

        continue;
      } // end IF

      break;
    } // end WHILE
    if (!pin_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: has no input pin, aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF

    result = builder_p->ConnectDirect (pin_p, pin_2, NULL);
    if (FAILED (result)) // 0x80040217: VFW_E_CANNOT_CONNECT, 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
    {
      std::list<std::wstring>::const_iterator iterator_2 = iterator;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::ConnectDirect() \"%s\" to \"%s\": \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*--iterator_2).c_str ()),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_2->Release ();
      enumerator_p->Release ();
      pin_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    pin_p->Release ();
    pin_2->Release ();
    pin_2 = NULL;

    result = enumerator_p->Reset ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IEnumPins::Reset(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      enumerator_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    pin_p = NULL;
    while (enumerator_p->Next (1, &pin_p, NULL) == S_OK)
    {
      ACE_ASSERT (pin_p);

      result = pin_p->QueryDirection (&pin_direction);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        pin_p->Release ();
        enumerator_p->Release ();
        builder_p->Release ();

        return false;
      } // end IF
      if (pin_direction != PINDIR_OUTPUT)
      {
        pin_p->Release ();
        pin_p = NULL;

        continue;
      } // end IF

      break;
    } // end WHILE
    enumerator_p->Release ();
  } // end FOR

  builder_p->Release ();

  return true;
}
bool
Stream_Module_Visualization_Tools::disconnect (ICaptureGraphBuilder2* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IGraphBuilder* builder_p = NULL;
  HRESULT result = builder_in->GetFiltergraph (&builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (builder_p);

  IEnumFilters* enumerator_p = NULL;
  result = builder_p->EnumFilters (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n")));

    // clean up
    builder_p->Release ();

    return false;
  } // end IF
  IBaseFilter* filter_p = NULL;
  IEnumPins* enumerator_2 = NULL;
  IPin* pin_p = NULL, *pin_2 = NULL;
  struct _FilterInfo filter_info;
  ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
  while (enumerator_p->Next (1, &filter_p, NULL) == S_OK)
  {
    ACE_ASSERT (filter_p);

    result = filter_p->QueryFilterInfo (&filter_info);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      enumerator_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF

    enumerator_2 = NULL;
    result = filter_p->EnumPins (&enumerator_2);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      enumerator_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (enumerator_2);

    while (enumerator_2->Next (1, &pin_p, NULL) == S_OK)
    {
      ACE_ASSERT (pin_p);

      pin_2 = NULL;
      result = pin_p->ConnectedTo (&pin_2);
      if (SUCCEEDED (result))
      {
        ACE_ASSERT (pin_2);

        result = pin_2->Disconnect ();
        if (FAILED (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPin::Disconnect(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));

          // clean up
          pin_2->Release ();
          pin_p->Release ();
          enumerator_2->Release ();
          filter_p->Release ();
          enumerator_p->Release ();
          builder_p->Release ();

          return false;
        } // end IF
        pin_2->Release ();
      
        result = pin_p->Disconnect ();
        if (FAILED (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPin::Disconnect(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));

          // clean up
          pin_p->Release ();
          enumerator_2->Release ();
          filter_p->Release ();
          enumerator_p->Release ();
          builder_p->Release ();

          return false;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("disconnected \"%s\"...\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName)));
      } // end IF

      pin_p->Release ();
      pin_p = NULL;
    } // end WHILE
    enumerator_2->Release ();

    filter_p->Release ();
    filter_p = NULL;
  } // end WHILE
  enumerator_p->Release ();

  builder_p->Release ();

  return true;
}
bool
Stream_Module_Visualization_Tools::getFormat (ICaptureGraphBuilder2* builder_in,
                                       struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::getFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (mediaType_out)
  {
    Stream_Module_Visualization_Tools::deleteMediaType (mediaType_out);
    mediaType_out = NULL;
  } // end IF

  IGraphBuilder* builder_p = NULL;
  HRESULT result = builder_in->GetFiltergraph (&builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (builder_p);

  IBaseFilter* filter_p = NULL;
  result =
    builder_p->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE,
                                 &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    builder_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  builder_p->Release ();

  IEnumPins* enumerator_p = NULL;
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  GUID GUID_i;
  ACE_OS::memset (&GUID_i, 0, sizeof (GUID));
  DWORD returned_size = 0;
  result = filter_p->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up   
    filter_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();

  while (enumerator_p->Next (1, &pin_p, NULL) == S_OK)
  {
    ACE_ASSERT (pin_p);

    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF
    if (pin_direction != PINDIR_OUTPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF
    property_set_p = NULL;
    result = pin_p->QueryInterface (IID_PPV_ARGS (&property_set_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryInterface(IKsPropertySet): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (property_set_p);
    result = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                                  NULL, 0,
                                  &GUID_i, sizeof (GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      property_set_p->Release ();
      pin_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (GUID));
    if (GUID_i == PIN_CATEGORY_CAPTURE)
      break;

    property_set_p->Release ();
    pin_p->Release ();
    pin_p = NULL;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no capture pin found, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE)));
    return false;
  } // end IF

  IAMStreamConfig* stream_config_p = NULL;
  result = pin_p->QueryInterface (IID_IAMStreamConfig,
                                  (void**)&stream_config_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  pin_p->Release ();
  ACE_ASSERT (stream_config_p);

  result = stream_config_p->GetFormat (&mediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    stream_config_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (mediaType_out);
  stream_config_p->Release ();

  return true;
}
bool
Stream_Module_Visualization_Tools::setFormat (ICaptureGraphBuilder2* builder_in,
                                       const AM_MEDIA_TYPE& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::setFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (builder_in);

  IGraphBuilder* builder_p = NULL;
  HRESULT result = builder_in->GetFiltergraph (&builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (builder_p);

  IGraphConfig* graph_config_p = NULL;
  result = builder_p->QueryInterface (IID_IGraphConfig,
                                      (void**)&graph_config_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IGraphConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    builder_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (graph_config_p);

  IBaseFilter* filter_p = NULL;
  result = builder_p->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE,
                                        &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(): \"%s\", aborting\n")));

    // clean up
    graph_config_p->Release ();
    builder_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  IEnumPins* enumerator_p = NULL;
  result = filter_p->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    graph_config_p->Release ();
    builder_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();

  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  GUID GUID_i;
  DWORD returned_size = 0;
  while (enumerator_p->Next (1, &pin_p, NULL) == S_OK)
  {
    ACE_ASSERT (pin_p);

    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();
      graph_config_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    if (pin_direction != PINDIR_OUTPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF
    property_set_p = NULL;
    result = pin_p->QueryInterface (IID_PPV_ARGS (&property_set_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryInterface(IKsPropertySet): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();
      graph_config_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (property_set_p);
    result = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                                  NULL, 0,
                                  &GUID_i, sizeof (GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      property_set_p->Release ();
      pin_p->Release ();
      enumerator_p->Release ();
      graph_config_p->Release ();
      builder_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (GUID));
    if (GUID_i == PIN_CATEGORY_CAPTURE)
      break;

    property_set_p->Release ();
    pin_p->Release ();
    pin_p = NULL;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no capture pin found, aborting\n")));

    // clean up
    graph_config_p->Release ();
    builder_p->Release ();

    return false;
  } // end IF

    //filter_p = NULL;
    //result = builder_p->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDERER,
    //                                      &filter_p);
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //  // clean up
    //  graph_config_p->Release ();
    //  builder_p->Release ();

    //  return false;
    //} // end IF
    //ACE_ASSERT (filter_p);
    //enumerator_p = NULL;
    //result = filter_p->EnumPins (&enumerator_p);
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //  // clean up
    //  filter_p->Release ();
    //  graph_config_p->Release ();
    //  builder_p->Release ();

    //  return false;
    //} // end IF
    //ACE_ASSERT (enumerator_p);
    //filter_p->Release ();

    //IPin* pin_2 = NULL;
    //while (enumerator_p->Next (1, &pin_2, NULL) == S_OK)
    //{
    //  ACE_ASSERT (pin_2);

    //  result = pin_2->QueryDirection (&pin_direction);
    //  if (FAILED (result))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
    //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //    // clean up
    //    pin_2->Release ();
    //    enumerator_p->Release ();
    //    pin_p->Release ();
    //    graph_config_p->Release ();
    //    builder_p->Release ();

    //    return false;
    //  } // end IF
    //  if (pin_direction == PINDIR_INPUT)
    //    break;

    //  pin_2->Release ();
    //  pin_2 = NULL;
    //} // end WHILE
    //enumerator_p->Release ();
    //if (!pin_2)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("no input pin found, aborting\n")));

    //  // clean up
    //  pin_p->Release ();
    //  graph_config_p->Release ();
    //  builder_p->Release ();

    //  return false;
    //} // end IF

  result = graph_config_p->Reconnect (pin_p,
                                      NULL,
                                      //pin_2,
                                      &mediaType_in,
                                      NULL,
                                      NULL,
                                      0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphConfig::Reconnect(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    //pin_2->Release ();
    pin_p->Release ();
    graph_config_p->Release ();
    builder_p->Release ();

    return false;
  } // end IF
    //pin_2->Release ();
  pin_p->Release ();
  graph_config_p->Release ();
  builder_p->Release ();

  return true;
}
void
Stream_Module_Visualization_Tools::deleteMediaType (struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::deleteMediaType"));

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

std::string
Stream_Module_Visualization_Tools::mediaSubTypeToString (const GUID& GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Visualization_Tools::mediaSubTypeToString"));

  std::string result;

  GUID2STRING_MAP_ITERATOR_T iterator =
    Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.find (GUID_in);
  if (iterator == Stream_Module_Visualization_Tools::Stream_MediaSubType2StringMap.end ())
  {
    OLECHAR GUID_string[39];
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    int nCount = StringFromGUID2 (GUID_in,
                                  GUID_string, sizeof (GUID_string));
    ACE_ASSERT (nCount == 39);
    ACE_Wide_To_Ascii converter (GUID_string);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (converter.char_rep ())));
    return result;
  } // end IF
  result = (*iterator).second;

  return result;
}
#endif
