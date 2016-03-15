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

#include "stream_dev_tools.h"

#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dvdmedia.h"
//#include "ksuuids.h"
#include "mtype.h"
#include "strmif.h"
#include "qedit.h"
#include "wmcodecdsp.h"
#endif

#include "common_tools.h"

#include "stream_macros.h"

#include "stream_dev_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// initialize statics
Stream_Module_Device_Tools::GUID2STRING_MAP_T Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap;
Stream_Module_Device_Tools::GUID2STRING_MAP_T Stream_Module_Device_Tools::Stream_MediaSubType2StringMap;
Stream_Module_Device_Tools::GUID2STRING_MAP_T Stream_Module_Device_Tools::Stream_FormatType2StringMap;
ACE_HANDLE Stream_Module_Device_Tools::logFileHandle = ACE_INVALID_HANDLE;
#endif

void
Stream_Module_Device_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initialize"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Video, ACE_TEXT_ALWAYS_CHAR ("vids")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Audio, ACE_TEXT_ALWAYS_CHAR ("auds")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Text, ACE_TEXT_ALWAYS_CHAR ("txts")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Midi, ACE_TEXT_ALWAYS_CHAR ("mids")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Stream, ACE_TEXT_ALWAYS_CHAR ("Stream")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Interleaved, ACE_TEXT_ALWAYS_CHAR ("iavs")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_File, ACE_TEXT_ALWAYS_CHAR ("file")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_ScriptCommand, ACE_TEXT_ALWAYS_CHAR ("scmd")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_AUXLine21Data, ACE_TEXT_ALWAYS_CHAR ("AUXLine21Data")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_AUXTeletextPage, ACE_TEXT_ALWAYS_CHAR ("AUXTeletextPage")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_CC_CONTAINER, ACE_TEXT_ALWAYS_CHAR ("CC_CONTAINER")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_DTVCCData, ACE_TEXT_ALWAYS_CHAR ("DTVCCData")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MSTVCaption, ACE_TEXT_ALWAYS_CHAR ("MSTVCaption")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_VBI, ACE_TEXT_ALWAYS_CHAR ("VBI")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Timecode, ACE_TEXT_ALWAYS_CHAR ("Timecode")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_LMRT, ACE_TEXT_ALWAYS_CHAR ("lmrt")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_URL_STREAM, ACE_TEXT_ALWAYS_CHAR ("URL_STREAM")));

  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PACK, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PACK")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PES, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PES")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_SECTIONS, ACE_TEXT_ALWAYS_CHAR ("MPEG2_SECTIONS")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PACK, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PACK")));

  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_DVD_ENCRYPTED_PACK, ACE_TEXT_ALWAYS_CHAR ("DVD_ENCRYPTED_PACK")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_DVD_NAVIGATION, ACE_TEXT_ALWAYS_CHAR ("DVD_NAVIGATION")));

  /////////////////////////////////////// AUDIO
  // uncompressed audio
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));

  // MPEG-4 and AAC
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_ADTS_AAC")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR("MPEG_HEAAC")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_LOAS, ACE_TEXT_ALWAYS_CHAR("MPEG_LOAS")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_AAC1, ACE_TEXT_ALWAYS_CHAR("RAW_AAC1")));

  // Dolby
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_DDPLUS, ACE_TEXT_ALWAYS_CHAR("DOLBY_DDPLUS")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVM, ACE_TEXT_ALWAYS_CHAR("DVM")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_SPORT, ACE_TEXT_ALWAYS_CHAR("RAW_SPORT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_SPDIF_TAG_241h, ACE_TEXT_ALWAYS_CHAR("SPDIF_TAG_241h")));

  // miscellaneous
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DRM_Audio, ACE_TEXT_ALWAYS_CHAR("DRM_Audio")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS2, ACE_TEXT_ALWAYS_CHAR("DTS2")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_PCMAudio_Obsolete, ACE_TEXT_ALWAYS_CHAR("PCMAudio_Obsolete")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_RAW_AAC")));

  /////////////////////////////////////// BDA
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_None, ACE_TEXT_ALWAYS_CHAR("None")));

  /////////////////////////////////////// DVD
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_SUBPICTURE, ACE_TEXT_ALWAYS_CHAR("DVD_SUBPICTURE")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_SDDS, ACE_TEXT_ALWAYS_CHAR("SDDS")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_DSI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_DSI")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PCI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PCI")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PROVIDER")));

  /////////////////////////////////////// Line 21
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_BytePair, ACE_TEXT_ALWAYS_CHAR("Line21_BytePair")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_GOPPacket, ACE_TEXT_ALWAYS_CHAR("Line21_GOPPacket")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_VBIRawData, ACE_TEXT_ALWAYS_CHAR("Line21_VBIRawData")));

  /////////////////////////////////////// MPEG-1
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));

  /////////////////////////////////////// MPEG-2
  // MPEG-2 (splitter)
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_VIDEO, ACE_TEXT_ALWAYS_CHAR("MPEG2_VIDEO")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  // MPEG-2 (demultiplexer)
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_PROGRAM, ACE_TEXT_ALWAYS_CHAR("MPEG2_PROGRAM")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT_STRIDE")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ATSC_SI, ACE_TEXT_ALWAYS_CHAR("ATSC_SI")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVB_SI, ACE_TEXT_ALWAYS_CHAR("DVB_SI")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ISDB_SI, ACE_TEXT_ALWAYS_CHAR("ISDB_SI")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2DATA, ACE_TEXT_ALWAYS_CHAR("MPEG2DATA")));
  // MPEG-2 (kernel)

  /////////////////////////////////////// Stream
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AIFF, ACE_TEXT_ALWAYS_CHAR("AIFF")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Asf, ACE_TEXT_ALWAYS_CHAR("Asf")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Avi, ACE_TEXT_ALWAYS_CHAR("Avi")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AU, ACE_TEXT_ALWAYS_CHAR("AU")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DssAudio, ACE_TEXT_ALWAYS_CHAR("DssAudio")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DssVideo, ACE_TEXT_ALWAYS_CHAR("DssVideo")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1SystemStream, ACE_TEXT_ALWAYS_CHAR("MPEG1SystemStream")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WAVE, ACE_TEXT_ALWAYS_CHAR("WAVE")));

  /////////////////////////////////////// VBI
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_RAW8, ACE_TEXT_ALWAYS_CHAR("RAW8")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_TELETEXT, ACE_TEXT_ALWAYS_CHAR("TELETEXT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPS, ACE_TEXT_ALWAYS_CHAR("VPS")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WSS, ACE_TEXT_ALWAYS_CHAR("WSS")));

  /////////////////////////////////////// VIDEO
  // analog video
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_NTSC_M, ACE_TEXT_ALWAYS_CHAR("NTSC_M")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_B, ACE_TEXT_ALWAYS_CHAR("PAL_B")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_D, ACE_TEXT_ALWAYS_CHAR("PAL_D")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_G, ACE_TEXT_ALWAYS_CHAR("PAL_G")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_H, ACE_TEXT_ALWAYS_CHAR("PAL_H")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_I, ACE_TEXT_ALWAYS_CHAR("PAL_I")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_M, ACE_TEXT_ALWAYS_CHAR("PAL_M")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_N, ACE_TEXT_ALWAYS_CHAR("PAL_N")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_B, ACE_TEXT_ALWAYS_CHAR("SECAM_B")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_D, ACE_TEXT_ALWAYS_CHAR("SECAM_D")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_G, ACE_TEXT_ALWAYS_CHAR("SECAM_G")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_H, ACE_TEXT_ALWAYS_CHAR("SECAM_H")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K, ACE_TEXT_ALWAYS_CHAR("SECAM_K")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K1, ACE_TEXT_ALWAYS_CHAR("SECAM_K1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_L, ACE_TEXT_ALWAYS_CHAR("SECAM_L")));

  // directx video acceleration
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AI44, ACE_TEXT_ALWAYS_CHAR("AI44")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IA44, ACE_TEXT_ALWAYS_CHAR("IA44")));

  // DV video
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvsl, ACE_TEXT_ALWAYS_CHAR("dvsl")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvsd, ACE_TEXT_ALWAYS_CHAR("dvsd")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_dvhd, ACE_TEXT_ALWAYS_CHAR("dvhd")));

  // H.264
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AVC1, ACE_TEXT_ALWAYS_CHAR("AVC1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_H264, ACE_TEXT_ALWAYS_CHAR("H264")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_h264, ACE_TEXT_ALWAYS_CHAR("h264")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_X264, ACE_TEXT_ALWAYS_CHAR("X264")));
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_x264, ACE_TEXT_ALWAYS_CHAR("x264")));

  // uncompressed RGB (no alpha)
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB1, ACE_TEXT_ALWAYS_CHAR("RGB1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB4, ACE_TEXT_ALWAYS_CHAR("RGB4")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB8, ACE_TEXT_ALWAYS_CHAR("RGB8")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB555, ACE_TEXT_ALWAYS_CHAR("RGB555")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB565, ACE_TEXT_ALWAYS_CHAR("RGB565")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB24, ACE_TEXT_ALWAYS_CHAR("RGB24")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32, ACE_TEXT_ALWAYS_CHAR("RGB32")));
  // uncompressed RGB (alpha)
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555, ACE_TEXT_ALWAYS_CHAR("ARGB1555")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32, ACE_TEXT_ALWAYS_CHAR("ARGB32")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444, ACE_TEXT_ALWAYS_CHAR("ARGB4444")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_A2R10G10B10, ACE_TEXT_ALWAYS_CHAR("A2R10G10B10")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_A2B10G10R10, ACE_TEXT_ALWAYS_CHAR("A2B10G10R10")));

  // video mixing renderer (VMR-7)
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX7_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX7_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX7_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX7_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX7_RT")));
  // video mixing renderer (VMR-9)
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX9_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX9_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX9_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX9_RT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX9_RT")));

  // YUV video
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_AYUV, ACE_TEXT_ALWAYS_CHAR("AYUV")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YUY2, ACE_TEXT_ALWAYS_CHAR("YUY2")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_UYVY, ACE_TEXT_ALWAYS_CHAR("UYVY")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC1, ACE_TEXT_ALWAYS_CHAR("IMC1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC2, ACE_TEXT_ALWAYS_CHAR("IMC2")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC3, ACE_TEXT_ALWAYS_CHAR("IMC3")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IMC4, ACE_TEXT_ALWAYS_CHAR("IMC4")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YV12, ACE_TEXT_ALWAYS_CHAR("YV12")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_NV12, ACE_TEXT_ALWAYS_CHAR("NV12")));
  // other YUV
  //Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_I420, ACE_TEXT_ALWAYS_CHAR("I420")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IF09, ACE_TEXT_ALWAYS_CHAR("IF09")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IYUV, ACE_TEXT_ALWAYS_CHAR("IYUV")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y211, ACE_TEXT_ALWAYS_CHAR("Y211")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y411, ACE_TEXT_ALWAYS_CHAR("Y411")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Y41P, ACE_TEXT_ALWAYS_CHAR("Y41P")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YVU9, ACE_TEXT_ALWAYS_CHAR("YVU9")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YVYU, ACE_TEXT_ALWAYS_CHAR("YVYU")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_YUYV, ACE_TEXT_ALWAYS_CHAR("YUYV")));

  // miscellaneous
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CFCC, ACE_TEXT_ALWAYS_CHAR("CFCC")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CLJR, ACE_TEXT_ALWAYS_CHAR("CLJR")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CPLA, ACE_TEXT_ALWAYS_CHAR("CPLA")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_CLPL, ACE_TEXT_ALWAYS_CHAR("CLPL")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_IJPG, ACE_TEXT_ALWAYS_CHAR("IJPG")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MDVF, ACE_TEXT_ALWAYS_CHAR("MDVF")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_MJPG, ACE_TEXT_ALWAYS_CHAR("MJPG")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Overlay, ACE_TEXT_ALWAYS_CHAR("Overlay")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_Plum, ACE_TEXT_ALWAYS_CHAR("Plum")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTJpeg, ACE_TEXT_ALWAYS_CHAR("QTJpeg")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTMovie, ACE_TEXT_ALWAYS_CHAR("QTMovie")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTRle, ACE_TEXT_ALWAYS_CHAR("QTRle")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTRpza, ACE_TEXT_ALWAYS_CHAR("QTRpza")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_QTSmc, ACE_TEXT_ALWAYS_CHAR("QTSmc")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_TVMJ, ACE_TEXT_ALWAYS_CHAR("TVMJ")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPVBI, ACE_TEXT_ALWAYS_CHAR("VPVBI")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_VPVideo, ACE_TEXT_ALWAYS_CHAR("VPVideo")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_WAKE, ACE_TEXT_ALWAYS_CHAR("WAKE")));

  ///////////////////////////////////////
  // unknown
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVCS, ACE_TEXT_ALWAYS_CHAR("DVCS")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MEDIASUBTYPE_DVSD, ACE_TEXT_ALWAYS_CHAR("DVSD")));

  // ---------------------------------------------------------------------------

  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_None, ACE_TEXT_ALWAYS_CHAR ("None")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_VideoInfo, ACE_TEXT_ALWAYS_CHAR ("VideoInfo")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_VideoInfo2, ACE_TEXT_ALWAYS_CHAR ("VideoInfo2")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_WaveFormatEx, ACE_TEXT_ALWAYS_CHAR ("WaveFormatEx")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEGVideo, ACE_TEXT_ALWAYS_CHAR ("MPEGVideo")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEGStreams, ACE_TEXT_ALWAYS_CHAR ("MPEGStreams")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_DvInfo, ACE_TEXT_ALWAYS_CHAR ("DvInfo")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_525WSS, ACE_TEXT_ALWAYS_CHAR ("525WSS")));

  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEG2_VIDEO, ACE_TEXT_ALWAYS_CHAR ("MPEG2_VIDEO")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_VIDEOINFO2, ACE_TEXT_ALWAYS_CHAR ("VIDEOINFO2")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEG2Video, ACE_TEXT_ALWAYS_CHAR ("MPEG2Video")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_DolbyAC3, ACE_TEXT_ALWAYS_CHAR ("DolbyAC3")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_MPEG2Audio, ACE_TEXT_ALWAYS_CHAR ("MPEG2Audio")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_DVD_LPCMAudio, ACE_TEXT_ALWAYS_CHAR ("DVD_LPCMAudio")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_UVCH264Video, ACE_TEXT_ALWAYS_CHAR ("UVCH264Video")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_JPEGImage, ACE_TEXT_ALWAYS_CHAR ("JPEGImage")));
  Stream_Module_Device_Tools::Stream_FormatType2StringMap.insert (std::make_pair (FORMAT_Image, ACE_TEXT_ALWAYS_CHAR ("Image")));
#endif
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
void
Stream_Module_Device_Tools::debug (IGraphBuilder* builder_in,
                                   const std::string& fileName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::debug"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (Stream_Module_Device_Tools::logFileHandle != ACE_INVALID_HANDLE)
    goto continue_;

  if (!fileName_in.empty ())
  {
    Stream_Module_Device_Tools::logFileHandle =
      ACE_TEXT_CreateFile (ACE_TEXT (fileName_in.c_str ()),
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           CREATE_ALWAYS, // TRUNCATE_EXISTING :-)
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (Stream_Module_Device_Tools::logFileHandle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CreateFile(\"%s\"): \"%m\", returning\n"),
                  ACE_TEXT (fileName_in.c_str ())));
      return;
    } // end IF
  } // end IF

continue_:
  HRESULT result =
      builder_in->SetLogFile (((Stream_Module_Device_Tools::logFileHandle != ACE_INVALID_HANDLE) ? reinterpret_cast<DWORD_PTR> (Stream_Module_Device_Tools::logFileHandle)
                                                                                                 : NULL));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::SetLogFile(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT (fileName_in.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    if (!CloseHandle (Stream_Module_Device_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%m\", continuing\n")));

    return;
  } // end IF

  if (fileName_in.empty ())
  {
    // sanity check(s)
    ACE_ASSERT (Stream_Module_Device_Tools::logFileHandle != ACE_INVALID_HANDLE);

    if (!CloseHandle (Stream_Module_Device_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%m\", continuing\n")));
  } // end IF
}
void
Stream_Module_Device_Tools::dump (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (pin_in);

  IEnumMediaTypes* ienum_media_types_p = NULL;
  HRESULT result = pin_in->EnumMediaTypes (&ienum_media_types_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::EnumMediaTypes(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (ienum_media_types_p);

  struct _AMMediaType* media_types_a[1];
  ACE_OS::memset (media_types_a, 0, sizeof (media_types_a));
  ULONG fetched = 0;
  unsigned int index = 1;
  while (S_OK == ienum_media_types_p->Next (1,
                                            media_types_a,
                                            &fetched))
  {
    // sanity check(s)
    ACE_ASSERT (media_types_a[0]);

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("#%d: \"%s\"\n"),
                index,
                ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (*media_types_a[0]).c_str ())));

    Stream_Module_Device_Tools::deleteMediaType (media_types_a[0]);
    ++index;
  } // end WHILE

  ienum_media_types_p->Release ();
}

IPin*
Stream_Module_Device_Tools::pin (IBaseFilter* filter_in,
                                 enum _PinDirection direction_in)
{
  IPin* result = NULL;

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IEnumPins* enumerator_p = NULL;
  HRESULT result_2 = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (enumerator_p);

  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s;
  ACE_OS::memset (&GUID_s, 0, sizeof (struct _GUID));
  enum _PinDirection pin_direction;
  while (S_OK == enumerator_p->Next (1, &result, NULL))
  {
    ACE_ASSERT (result);

    result_2 = result->QueryDirection (&pin_direction);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

      // clean up
      result->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF
    if (pin_direction == direction_in)
      break;

    result->Release ();
    result = NULL;

    //property_set_p = NULL;
    //result = pin_p->QueryInterface (IID_PPV_ARGS (&property_set_p));
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //    ACE_TEXT ("failed to IPin::QueryInterface(IKsPropertySet): \"%s\", aborting\n"),
    //    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //  // clean up
    //  pin_p->Release ();
    //  enumerator_p->Release ();

    //  goto error;
    //} // end IF
    //ACE_ASSERT (property_set_p);
    //result = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
    //  NULL, 0,
    //  &GUID_s, sizeof (struct _GUID), &returned_size);
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //    ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
    //    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //  // clean up
    //  property_set_p->Release ();
    //  pin_p->Release ();
    //  enumerator_p->Release ();

    //  goto error;
    //} // end IF
    //ACE_ASSERT (returned_size == sizeof (struct _GUID));
    //if (GUID_s == PIN_CATEGORY_CAPTURE)
    //  break;

    //property_set_p->Release ();
    //pin_p->Release ();
    //pin_p = NULL;
  } // end WHILE
  enumerator_p->Release ();

  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("0x%@: no pin found (direction was: %d), aborting\n"),
                filter_in,
                direction_in));
    return NULL;
  } // end IF

  return result;
}

IBaseFilter*
Stream_Module_Device_Tools::pin2Filter (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::pin2Filter"));

  // sanity check(s)
  ACE_ASSERT (pin_in);

  struct _PinInfo pin_info;
  ACE_OS::memset (&pin_info, 0, sizeof (struct _PinInfo));
  HRESULT result = pin_in->QueryPinInfo (&pin_info);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryPinInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (pin_info.pFilter);

  return pin_info.pFilter;
}

std::string
Stream_Module_Device_Tools::name (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::name"));

  std::string result;

  struct _FilterInfo filter_info;
  ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
  HRESULT result_2 = filter_in->QueryFilterInfo (&filter_info);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return result;
  } // end iF
  result =
    ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName));

  // clean up
  if (filter_info.pGraph)
    filter_info.pGraph->Release ();

  return result;
}

bool
Stream_Module_Device_Tools::loadDeviceGraph (const std::string& deviceName_in,
                                             IGraphBuilder*& IGraphBuilder_inout,
                                             IAMBufferNegotiation*& IAMBufferNegotiation_out,
                                             IAMStreamConfig*& IAMStreamConfig_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::loadDeviceGraph"));

  // sanity check(s)
  if (IAMBufferNegotiation_out)
  {
    IAMBufferNegotiation_out->Release ();
    IAMBufferNegotiation_out = NULL;
  } // end IF
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release ();
    IAMStreamConfig_out = NULL;
  } // end IF

  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;
  if (!IGraphBuilder_inout)
  {
    ICaptureGraphBuilder2* builder_2 = NULL;
    result =
      CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL,
                        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2,
                        (void**)&builder_2);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_CaptureGraphBuilder2): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (builder_2);

    result = CoCreateInstance (CLSID_FilterGraph, NULL,
                               CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
                               (void**)&IGraphBuilder_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      builder_2->Release ();

      return false;
    } // end IF
    ACE_ASSERT (IGraphBuilder_inout);

    result = builder_2->SetFiltergraph (IGraphBuilder_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ICaptureGraphBuilder2::SetFiltergraph(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      builder_2->Release ();

      goto error;
    } // end IF
    builder_2->Release ();
  } // end IF
  else
  {
    if (!Stream_Module_Device_Tools::clear (IGraphBuilder_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (IGraphBuilder_inout);

  ICreateDevEnum* enumerator_p = NULL;
  result =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
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

    //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    goto error;
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

      goto error;
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

      goto error;
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
    goto error;
  } // end IF

  result = moniker_p->BindToObject (0, 0, IID_IBaseFilter,
                                    (void**)&filter_p);
  moniker_p->Release ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMoniker::BindToObject(IID_IBaseFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result =
    IGraphBuilder_inout->AddFilter (filter_p,
                                    MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();

    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO)));

  IEnumPins* enumerator_2 = NULL;
  result = filter_p->EnumPins (&enumerator_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();

    goto error;
  } // end IF
  ACE_ASSERT (enumerator_2);
  filter_p->Release ();

  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s;
  ACE_OS::memset (&GUID_s, 0, sizeof (struct _GUID));
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

      goto error;
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

      goto error;
    } // end IF
    ACE_ASSERT (property_set_p);
    result = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                                  NULL, 0,
                                  &GUID_s, sizeof (struct _GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      property_set_p->Release ();
      pin_p->Release ();
      enumerator_p->Release ();

      goto error;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (struct _GUID));
    if (GUID_s == PIN_CATEGORY_CAPTURE)
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
    goto error;
  } // end IF

  result = pin_p->QueryInterface (IID_IAMBufferNegotiation,
                                  (void**)&IAMBufferNegotiation_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMBufferNegotiation): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    goto error;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);

  result = pin_p->QueryInterface (IID_IAMStreamConfig,
                                  (void**)&IAMStreamConfig_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    goto error;
  } // end IF
  ACE_ASSERT (IAMStreamConfig_out);
  pin_p->Release ();

  return true;

error:
  if (IGraphBuilder_inout)
  {
    IGraphBuilder_inout->Release ();
    IGraphBuilder_inout = NULL;
  } // end IF
  if (IAMBufferNegotiation_out)
  {
    IAMBufferNegotiation_out->Release ();
    IAMBufferNegotiation_out = NULL;
  } // end IF
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release ();
    IAMStreamConfig_out = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Device_Tools::loadRendererGraph (const struct _AMMediaType& mediaType_in,
                                               const HWND windowHandle_in,
                                               IGraphBuilder* IGraphBuilder_in,
                                               std::list<std::wstring>& pipeline_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::loadRendererGraph"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  pipeline_out.clear ();

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  //if (!IGraphBuilder_out)
  //{
  //  result =
  //    CoCreateInstance (CLSID_FilterGraph, NULL,
  //                      CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
  //                      (void**)&IGraphBuilder_out);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    return false;
  //  } // end IF
  //  ACE_ASSERT (IGraphBuilder_out);
  //} // end IF
  //else
  //{
    if (!Stream_Module_Device_Tools::resetDeviceGraph (IGraphBuilder_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::resetDeviceGraph(), aborting\n")));
      return false;
    } // end IF
  //} // end ELSE
  //ACE_ASSERT (IGraphBuilder_out);

  OLECHAR GUID_string[39];
  ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
  int count = 0;

  // convert RGB / YUV / MJPEG --> RGB ?
  struct _GUID converter_CLSID = CLSID_Colour;
  std::wstring converter_name = MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_RGB;
  if (mediaType_in.subtype == MEDIASUBTYPE_MJPG)
  {
    converter_CLSID = CLSID_MjpegDec;
    converter_name = MODULE_DEV_CAM_WIN32_FILTER_NAME_DECOMPRESS_MJPG;
  } // end IF
  else if (mediaType_in.subtype == MEDIASUBTYPE_YUY2)
  {
    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
    converter_CLSID = CLSID_AVIDec;
    converter_name = MODULE_DEV_CAM_WIN32_FILTER_NAME_DECOMPRESS_AVI;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (mediaType_in.subtype).c_str ())));
    return false;
  } // end IF

  IBaseFilter* filter_p = NULL;
  result = CoCreateInstance (converter_CLSID, NULL,
                             CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                             (void**)&filter_p);
  if (FAILED (result))
  {
    count = StringFromGUID2 (converter_CLSID,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  result = IGraphBuilder_in->AddFilter (filter_p,
                                        converter_name.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (converter_name.c_str ())));

  IBaseFilter* filter_2 = NULL;
  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                             (void**)&filter_2);
  if (FAILED (result))
  {
    count = StringFromGUID2 (converter_CLSID,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_2);
  result = IGraphBuilder_in->AddFilter (filter_2,
                                        MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB)));

  // render to a window (GtkDrawingArea) ?
  IBaseFilter* filter_3 = NULL;
  result = CoCreateInstance ((windowHandle_in ? CLSID_VideoRenderer
                                              : CLSID_NullRenderer), NULL,
                             CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                             (void**)&filter_3);
  if (FAILED (result))
  {
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    count = StringFromGUID2 ((windowHandle_in ? CLSID_VideoRenderer
                                              : CLSID_NullRenderer),
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_3);
  result =
    IGraphBuilder_in->AddFilter (filter_3,
                                 (windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                                  : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                                        : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL))));

  //result =
  //  ICaptureGraphBuilder2_in->RenderStream (//&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
  //                                          &PIN_CATEGORY_CAPTURE, NULL,
  //                                          filter_p,
  //                                          filter_2,
  //                                          //NULL,
  //                                          filter_4);
  //if (FAILED (result)) // E_INVALIDARG = 0x80070057, 0x80040217 = VFW_E_CANNOT_CONNECT ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ICaptureGraphBuilder::RenderStream(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return false;
  //} // end IF

  pipeline_out.push_back (converter_name);
  pipeline_out.push_back (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB);
  pipeline_out.push_back ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                           : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL));

  // clean up
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();

  return true;

error:
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();

  return false;
}
bool
Stream_Module_Device_Tools::loadTargetRendererGraph (const HWND windowHandle_in,
                                                     IGraphBuilder*& IGraphBuilder_out,
                                                     std::list<std::wstring>& pipeline_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::loadTargetRendererGraph"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  pipeline_out.clear ();

  if (!IGraphBuilder_out)
  {
    result =
      CoCreateInstance (CLSID_FilterGraph, NULL,
                        CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
                        (void**)&IGraphBuilder_out);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (IGraphBuilder_out);
  } // end IF
  else
  {
    if (!Stream_Module_Device_Tools::clear (IGraphBuilder_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (IGraphBuilder_out);

  //// split
  OLECHAR GUID_string[39];
  IBaseFilter* filter_p = NULL;
  //result =
  //  IGraphBuilder_out->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_SPLIT_AVI,
  //                                       &filter_p);
  //if (FAILED (result))
  //{
  //  if (result != VFW_E_NOT_FOUND)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
  //                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_SPLIT_AVI),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    goto error;
  //  } // end IF

  //  // *TODO*: support receiving other formats
  //  result = CoCreateInstance (CLSID_AVIDec, NULL,
  //                             CLSCTX_INPROC_SERVER, IID_IBaseFilter,
  //                             (void**)&filter_p);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to CoCreateInstance() AVI decompressor: \"%s\", aborting\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    goto error;
  //  } // end IF
  //  ACE_ASSERT (filter_p);
  //  result =
  //    IGraphBuilder_out->AddFilter (filter_p,
  //                                  MODULE_DEV_CAM_WIN32_FILTER_NAME_SPLIT_AVI);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    goto error;
  //  } // end IF
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("added \"%s\"...\n"),
  //              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_SPLIT_AVI)));
  //} // end IF
  //ACE_ASSERT (filter_p);

  // convert RGB
  IBaseFilter* filter_2 = NULL;
  result =
    IGraphBuilder_out->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_RGB,
                                         &filter_2);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_RGB),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    // *TODO*: support receiving other formats
    result = CoCreateInstance (CLSID_Colour, NULL,
                               CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                               (void**)&filter_2);
    if (FAILED (result))
    {
      ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
      int nCount =
        StringFromGUID2 (CLSID_Colour,
                         GUID_string, sizeof (GUID_string));
      ACE_ASSERT (nCount == 39);

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_2);
    result =
      IGraphBuilder_out->AddFilter (filter_2,
                                    MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_RGB);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_RGB)));
  } // end IF
  ACE_ASSERT (filter_2);

  // render to a window (GtkDrawingArea) ?
  IBaseFilter* filter_3 = NULL;
  result =
    IGraphBuilder_out->FindFilterByName ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                                          : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL),
                                         &filter_3);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                                            : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL)),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance ((windowHandle_in ? CLSID_VideoRenderer
                                                : CLSID_NullRenderer), NULL,
                               CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                               (void**)&filter_3);
    if (FAILED (result))
    {
      ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
      int nCount =
        StringFromGUID2 ((windowHandle_in ? CLSID_VideoRenderer
                                          : CLSID_NullRenderer),
                         GUID_string, sizeof (GUID_string));
      ACE_ASSERT (nCount == 39);

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_3);
    result =
      IGraphBuilder_out->AddFilter (filter_3,
                                    (windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                                     : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                                          : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL))));
  } // end IF
  ACE_ASSERT (filter_3);

  //result =
  //  ICaptureGraphBuilder2_in->RenderStream (//&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
  //                                          &PIN_CATEGORY_CAPTURE, NULL,
  //                                          filter_p,
  //                                          filter_2,
  //                                          //NULL,
  //                                          filter_4);
  //if (FAILED (result)) // E_INVALIDARG = 0x80070057, 0x80040217 = VFW_E_CANNOT_CONNECT ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ICaptureGraphBuilder::RenderStream(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return false;
  //} // end IF

  //pipeline_out.push_back (MODULE_DEV_CAM_WIN32_FILTER_NAME_SPLIT_AVI);
  pipeline_out.push_back (MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_RGB);
  pipeline_out.push_back ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO
                                           : MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL));

  // clean up
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();

  return true;

error:
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();

  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release ();
    IGraphBuilder_out = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Device_Tools::connect (IGraphBuilder* builder_in,
                                     const std::list<std::wstring>& graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::connect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!graph_in.empty ());

  IBaseFilter* filter_p = NULL;
  std::list<std::wstring>::const_iterator iterator = graph_in.begin ();
  HRESULT result =
    builder_in->FindFilterByName ((*iterator).c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IAMStreamConfig* stream_config_p = NULL;
  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
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

      return false;
    } // end IF
    if (pin_direction != PINDIR_OUTPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF
    //stream_config_p = NULL;
    //result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IPin::QueryInterface(IAMStreamConfig): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //  // clean up
    //  pin_p->Release ();
    //  enumerator_p->Release ();
    //  builder_p->Release ();

    //  return false;
    //} // end IF
    //ACE_ASSERT (stream_config_p);
    //result = stream_config_p->SetFormat (NULL);
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IAMStreamConfig::SetFormat(): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //  // clean up
    //  stream_config_p->Release ();
    //  pin_p->Release ();
    //  enumerator_p->Release ();
    //  builder_p->Release ();

    //  return false;
    //} // end IF
    //stream_config_p->Release ();

    break;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));
    return false;
  } // end IF
  IPin* pin_2 = NULL;
  //struct _PinInfo pin_info;
  //ACE_OS::memset (&pin_info, 0, sizeof (struct _PinInfo));
  std::list<std::wstring>::const_iterator iterator_2;
  for (++iterator;
       iterator != graph_in.end ();
       ++iterator)
  {
    filter_p = NULL;
    result =
      builder_in->FindFilterByName ((*iterator).c_str (),
                                    &filter_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();

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

      return false;
    } // end IF
    ACE_ASSERT (enumerator_p);
    filter_p->Release ();
    while (S_OK == enumerator_p->Next (1, &pin_2, NULL))
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

      return false;
    } // end IF

    iterator_2 = iterator;
    --iterator_2;
    //result = builder_p->ConnectDirect (pin_p, pin_2, NULL);

    result = pin_p->Connect (pin_2, NULL);
    if (FAILED (result)) // 0x80040217: VFW_E_CANNOT_CONNECT, 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
    {
      // *TODO*: evidently, some filters do not expose their preferred media
      //         types (e.g. AVI Splitter), so the straight-forward, 'direct'
      //         pin connection algorithm (as implemented here) will not
      //         always work. Note how (such as in this example), this
      //         actually makes some sense, as 'container'- or other 'meta-'
      //         filters sometimes actually do not know (or care) about what
      //         kind of data they contain
      // *NOTE*: 'fixing' this requires some in-depth knowledge about
      //         _AMMediaType (in-)compatibilities, and other inner workings of
      //         DirectShow (such as what the algorithm is that IGraphBuilder
      //         uses to intelligently retrieve 'pin'-compatible media types)...

      //if (result == VFW_E_NO_ACCEPTABLE_TYPES)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IPin::Connect() \"%s\", aborting\n"),
      //              ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));

      //  // debug info
      //  Stream_Module_Device_Tools::dump (pin_p);
      //  Stream_Module_Device_Tools::dump (pin_2);
      //} // end IF
      //else
      //{
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to IPin::Connect() \"%s\" to \"%s\": \"%s\" (0x%x), falling back\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ()),
                    result));
      //} // end ELSE

      result = builder_in->Connect (pin_p, pin_2);
      if (FAILED (result)) // 0x80040217: VFW_E_CANNOT_CONNECT, 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGraphBuilder::Connect() \"%s\" to \"%s\": \"%s\" (0x%x), aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ()),
                    result));
      } // end IF
      else
        goto continue_;

      // clean up
      pin_2->Release ();
      enumerator_p->Release ();
      pin_p->Release ();

      return false;
    } // end IF
continue_:
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connected \"%s\" to \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));
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

      return false;
    } // end IF
    pin_p = NULL;
    while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
    {
      // sanity check(s)
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

      break;
    } // end WHILE
    enumerator_p->Release ();
  } // end FOR

  return true;
}
bool
Stream_Module_Device_Tools::graphConnect (IGraphBuilder* builder_in,
                                          const std::list<std::wstring>& graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::graphConnect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!graph_in.empty ());

  IBaseFilter* filter_p = NULL;
  std::list<std::wstring>::const_iterator iterator = graph_in.begin ();
  HRESULT result =
    builder_in->FindFilterByName ((*iterator).c_str (),
      &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IAMStreamConfig* stream_config_p = NULL;
  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
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

      return false;
    } // end IF
    if (pin_direction != PINDIR_OUTPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF
      //stream_config_p = NULL;
      //result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IPin::QueryInterface(IAMStreamConfig): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      //  // clean up
      //  pin_p->Release ();
      //  enumerator_p->Release ();
      //  builder_p->Release ();

      //  return false;
      //} // end IF
      //ACE_ASSERT (stream_config_p);
      //result = stream_config_p->SetFormat (NULL);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IAMStreamConfig::SetFormat(): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      //  // clean up
      //  stream_config_p->Release ();
      //  pin_p->Release ();
      //  enumerator_p->Release ();
      //  builder_p->Release ();

      //  return false;
      //} // end IF
      //stream_config_p->Release ();

    break;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));
    return false;
  } // end IF
  IPin* pin_2 = NULL;
  //struct _PinInfo pin_info;
  //ACE_OS::memset (&pin_info, 0, sizeof (struct _PinInfo));
  std::list<std::wstring>::const_iterator iterator_2;
  for (++iterator;
       iterator != graph_in.end ();
       ++iterator)
  {
    filter_p = NULL;
    result = builder_in->FindFilterByName ((*iterator).c_str (),
                                           &filter_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();

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

      return false;
    } // end IF
    ACE_ASSERT (enumerator_p);
    filter_p->Release ();
    while (S_OK == enumerator_p->Next (1, &pin_2, NULL))
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

      return false;
    } // end IF

    iterator_2 = iterator;
    //result = builder_p->ConnectDirect (pin_p, pin_2, NULL);

    result = builder_in->Connect (pin_p, pin_2);
    if (FAILED (result)) // 0x80040217: VFW_E_CANNOT_CONNECT, 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
    {
      if (result == VFW_E_NO_ACCEPTABLE_TYPES)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGraphBuilder::Connect() \"%s\", aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));

        // debug info
        Stream_Module_Device_Tools::dump (pin_p);
        // *TODO*: evidently, some filters do not expose their preferred media
        //         types (e.g. AVI Splitter), so the straight-forward, 'direct'
        //         pin connection algorithm (as implemented here) will not
        //         always work. Note how (such as in this example), this
        //         actually makes some sense, as 'container'- or other 'meta-'
        //         filters sometimes actually do not know (or care) about what
        //         kind of data they contain
        Stream_Module_Device_Tools::dump (pin_2);
      } // end IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGraphBuilder::Connect() \"%s\" to \"%s\": \"%s\" (0x%x), aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*--iterator_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ()),
                    result));
      } // end ELSE

        // clean up
      pin_2->Release ();
      enumerator_p->Release ();
      pin_p->Release ();

      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connected \"%s\" to \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*--iterator_2).c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));
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

  return true;
}

bool
Stream_Module_Device_Tools::clear (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::clear"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IEnumFilters* enumerator_p = NULL;
  HRESULT result = builder_in->EnumFilters (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n")));
    return false;
  } // end IF
  IBaseFilter* filter_p = NULL;
  struct _FilterInfo filter_info;
  while (enumerator_p->Next (1, &filter_p, NULL) == S_OK)
  {
    ACE_ASSERT (filter_p);

    ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
    result = filter_p->QueryFilterInfo (&filter_info);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF

    // clean up
    if (filter_info.pGraph)
      filter_info.pGraph->Release ();

    result = builder_in->RemoveFilter (filter_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGrapBuilder::RemoveFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName)));

    result = enumerator_p->Reset ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IEnumFilters::Reset(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF

    filter_p->Release ();
    filter_p = NULL;
  } // end WHILE
  enumerator_p->Release ();

  return true;
}
bool
Stream_Module_Device_Tools::disconnect (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IEnumFilters* enumerator_p = NULL;
  HRESULT result = builder_in->EnumFilters (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n")));
    return false;
  } // end IF
  IBaseFilter* filter_p = NULL;
  IEnumPins* enumerator_2 = NULL;
  IPin* pin_p = NULL, *pin_2 = NULL;
  struct _FilterInfo filter_info;
  while (S_OK == enumerator_p->Next (1, &filter_p, NULL))
  {
    ACE_ASSERT (filter_p);

    ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
    result = filter_p->QueryFilterInfo (&filter_info);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF

    // clean up
    if (filter_info.pGraph)
      filter_info.pGraph->Release ();

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

      return false;
    } // end IF
    ACE_ASSERT (enumerator_2);

    while (S_OK == enumerator_2->Next (1, &pin_p, NULL))
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

  return true;
}
bool
Stream_Module_Device_Tools::resetDeviceGraph (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::resetDeviceGraph"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  if (!Stream_Module_Device_Tools::disconnect (builder_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), aborting\n")));
    return false;
  } // end IF

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO,
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  if (!Stream_Module_Device_Tools::clear (builder_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF

  result = builder_in->AddFilter (filter_p,
                                  MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  filter_p->Release ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO)));

  return true;
}

bool
Stream_Module_Device_Tools::getBufferNegotiation (IGraphBuilder* builder_in,
                                                  IAMBufferNegotiation*& IAMBufferNegotiation_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getBufferNegotiation"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (IAMBufferNegotiation_out)
  {
    IAMBufferNegotiation_out->Release ();
    IAMBufferNegotiation_out = NULL;
  } // end IF

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO,
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IPin* pin_p = Stream_Module_Device_Tools::pin (filter_p,
                                                 PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::pin(\"%s\",PINDIR_OUTPUT), aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::name (filter_p).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  filter_p->Release ();

  result = pin_p->QueryInterface (IID_IAMBufferNegotiation,
                                  (void**)&IAMBufferNegotiation_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMBufferNegotiation): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);
  pin_p->Release ();

  return true;
}

bool
Stream_Module_Device_Tools::getCaptureFormat (IGraphBuilder* builder_in,
                                              struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (mediaType_out)
  {
    Stream_Module_Device_Tools::deleteMediaType (mediaType_out);
    mediaType_out = NULL;
  } // end IF

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO,
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IEnumPins* enumerator_p = NULL;
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s;
  ACE_OS::memset (&GUID_s, 0, sizeof (struct _GUID));
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
                                  &GUID_s, sizeof (struct _GUID), &returned_size);
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
    ACE_ASSERT (returned_size == sizeof (struct _GUID));
    if (GUID_s == PIN_CATEGORY_CAPTURE)
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
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO)));
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
Stream_Module_Device_Tools::getOutputFormat (IGraphBuilder* builder_in,
                                             struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (mediaType_out)
  {
    Stream_Module_Device_Tools::deleteMediaType (mediaType_out);
    mediaType_out = NULL;
  } // end IF
  //mediaType_out = CreateMediaType (NULL);
  ACE_NEW_NORETURN (mediaType_out,
                    struct _AMMediaType ());
  if (!mediaType_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                //ACE_TEXT ("failed to CreateMediaType(): \"%m\", aborting\n")));
                ACE_TEXT ("failed to allocate memory(): \"%m\", aborting\n")));
    return false;
  } // end IF

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB,
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  ISampleGrabber* isample_grabber_p = NULL;
  result = filter_p->QueryInterface (IID_ISampleGrabber,
                                     (void**)&isample_grabber_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (isample_grabber_p);
  filter_p->Release ();
  filter_p = NULL;

  result = isample_grabber_p->GetConnectedMediaType (mediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ISampleGrabber::GetConnectedMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  isample_grabber_p->Release ();
  isample_grabber_p = NULL;

  return true;

error:
  if (isample_grabber_p)
    isample_grabber_p->Release ();
  if (filter_p)
    filter_p->Release ();

  if (mediaType_out)
  {
    Stream_Module_Device_Tools::deleteMediaType (mediaType_out);
    mediaType_out = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Device_Tools::setCaptureFormat (IGraphBuilder* builder_in,
                                              const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setCaptureFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (builder_in);

  //IGraphConfig* graph_config_p = NULL;
  //HRESULT result = builder_in->QueryInterface (IID_IGraphConfig,
  //                                             (void**)&graph_config_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IGraphConfig): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return false;
  //} // end IF
  //ACE_ASSERT (graph_config_p);

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO,
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //// clean up
    //graph_config_p->Release ();

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
    //graph_config_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();

  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s;
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
      //graph_config_p->Release ();

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
      //graph_config_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (property_set_p);
    result = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                                  NULL, 0,
                                  &GUID_s, sizeof (struct _GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      property_set_p->Release ();
      pin_p->Release ();
      enumerator_p->Release ();
      //graph_config_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (struct _GUID));
    if (GUID_s == PIN_CATEGORY_CAPTURE)
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
    //graph_config_p->Release ();

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
    return false;
  } // end IF
  ACE_ASSERT (stream_config_p);

  //result = graph_config_p->Reconnect (pin_p,
  //                                    NULL,
  //                                    //pin_2,
  //                                    &mediaType_in,
  //                                    NULL,
  //                                    NULL,
  //                                    0);
  result =
    stream_config_p->SetFormat (&const_cast<struct _AMMediaType&> (mediaType_in));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IGraphConfig::Reconnect(): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IAMStreamConfig::SetFormat(): \"%s\" (0x%x) (media type was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ()), result,
                ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (mediaType_in).c_str ())));

    // clean up
    stream_config_p->Release ();
    //pin_2->Release ();
    pin_p->Release ();
    //graph_config_p->Release ();

    return false;
  } // end IF
  stream_config_p->Release ();
  //pin_2->Release ();
  pin_p->Release ();
  //graph_config_p->Release ();

  return true;
}
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
void
Stream_Module_Device_Tools::freeMediaType (struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::freeMediaType"));

  if (mediaType_in.cbFormat != 0)
  {
    CoTaskMemFree ((PVOID)mediaType_in.pbFormat);
    mediaType_in.cbFormat = 0;
    mediaType_in.pbFormat = NULL;
  } // end IF
  if (mediaType_in.pUnk != NULL)
  {
    // pUnk should not be used.
    mediaType_in.pUnk->Release ();
    mediaType_in.pUnk = NULL;
  } // end IF
}

std::string
Stream_Module_Device_Tools::mediaSubTypeToString (const struct _GUID& GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::mediaSubTypeToString"));

  std::string result;

  GUID2STRING_MAP_ITERATOR_T iterator =
    Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.find (GUID_in);
  if (iterator == Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.end ())
  {
    OLECHAR GUID_string[39];
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    int nCount = StringFromGUID2 (GUID_in,
                                  GUID_string, sizeof (GUID_string));
    ACE_ASSERT (nCount == 39);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    return result;
  } // end IF
  result = (*iterator).second;

  return result;
}

std::string
Stream_Module_Device_Tools::mediaTypeToString (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::mediaTypeToString"));

  std::string result;

  OLECHAR GUID_string[39];
  ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
  int count = -1;
  GUID2STRING_MAP_ITERATOR_T iterator =
    Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.find (mediaType_in.majortype);
  if (iterator == Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.end ())
  {
    count = StringFromGUID2 (mediaType_in.majortype,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media majortype (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    return std::string ();
  } // end IF
  result = ACE_TEXT_ALWAYS_CHAR ("majortype: \"");
  result += (*iterator).second;
  result += ACE_TEXT_ALWAYS_CHAR ("\"\nsubtype: \"");
  iterator =
    Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.find (mediaType_in.subtype);
  if (iterator == Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.end ())
  {
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    count = StringFromGUID2 (mediaType_in.subtype,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    return std::string ();
  } // end IF
  result += (*iterator).second;

  result += ACE_TEXT_ALWAYS_CHAR ("\"\nbFixedSizeSamples: ");
  std::ostringstream converter;
  converter << mediaType_in.bFixedSizeSamples;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\nbTemporalCompression: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.bTemporalCompression;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\nlSampleSize: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.lSampleSize;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\nformattype: \"");
  iterator =
    Stream_Module_Device_Tools::Stream_FormatType2StringMap.find (mediaType_in.formattype);
  if (iterator == Stream_Module_Device_Tools::Stream_FormatType2StringMap.end ())
  {
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    count = StringFromGUID2 (mediaType_in.formattype,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    return std::string ();
  } // end IF
  result += (*iterator).second;

  result += ACE_TEXT_ALWAYS_CHAR ("\"\npUnk: 0x");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << std::hex << mediaType_in.pUnk << std::dec;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\ncbFormat: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.cbFormat;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\npbFormat: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << std::hex << mediaType_in.pbFormat << std::dec;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  if (mediaType_in.formattype == FORMAT_VideoInfo)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result += ACE_TEXT_ALWAYS_CHAR ("---\nrcSource [lrtb]: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->rcSource.left;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header_p->rcSource.right;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header_p->rcSource.top;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header_p->rcSource.bottom;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nrcTarget [lrtb]: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->rcTarget.left;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header_p->rcTarget.right;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header_p->rcTarget.top;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header_p->rcTarget.bottom;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwBitRate: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->dwBitRate;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwBitErrorRate: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->dwBitErrorRate;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nAvgTimePerFrame: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->AvgTimePerFrame;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\n---\nbiSize: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biSize;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiWidth: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biWidth;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiHeight: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biHeight;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiPlanes: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biPlanes;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiBitCount: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biBitCount;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiCompression: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biCompression;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiSizeImage: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biSizeImage;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiXPelsPerMeter: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biXPelsPerMeter;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiYPelsPerMeter: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biYPelsPerMeter;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiClrUsed: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biClrUsed;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiClrImportant: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biClrImportant;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end IF
  else if (mediaType_in.formattype == FORMAT_VideoInfo2)
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result += ACE_TEXT_ALWAYS_CHAR ("---\nrcSource [lrtb]: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->rcSource.left;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header2_p->rcSource.right;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header2_p->rcSource.top;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header2_p->rcSource.bottom;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nrcTarget [lrtb]: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->rcTarget.left;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header2_p->rcTarget.right;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header2_p->rcTarget.top;
    converter << ACE_TEXT_ALWAYS_CHAR (",");
    converter << video_info_header2_p->rcTarget.bottom;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwBitRate: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwBitRate;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwBitErrorRate: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwBitErrorRate;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nAvgTimePerFrame: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->AvgTimePerFrame;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwInterlaceFlags: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwInterlaceFlags;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwCopyProtectFlags: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwCopyProtectFlags;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwPictAspectRatioX: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwPictAspectRatioX;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwPictAspectRatioY: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwPictAspectRatioY;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwControlFlags: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwControlFlags;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwReserved2: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwReserved2;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\n---\nbiSize: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biSize;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiWidth: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biWidth;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiHeight: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biHeight;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiPlanes: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biPlanes;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiBitCount: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biBitCount;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiCompression: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biCompression;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiSizeImage: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biSizeImage;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiXPelsPerMeter: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biXPelsPerMeter;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiYPelsPerMeter: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biYPelsPerMeter;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiClrUsed: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biClrUsed;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nbiClrImportant: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biClrImportant;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end ELSE IF
  else
  {
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    count = StringFromGUID2 (mediaType_in.formattype,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == 39);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
  } // end ELSE

  return result;
}
#else
bool
Stream_Module_Device_Tools::canOverlay (int fd_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::canOverlay"));

  int result = -1;

  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return false;
  } // end IF

  return (device_capabilities.device_caps & V4L2_CAP_VIDEO_OVERLAY);
}
bool
Stream_Module_Device_Tools::canStream (int fd_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::canStream"));

  int result = -1;

  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return false;
  } // end IF

  return (device_capabilities.device_caps & V4L2_CAP_STREAMING);
}
void
Stream_Module_Device_Tools::dump (int fd_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::dump"));

  int result = -1;

  // sanity check(s)
  struct v4l2_capability device_capabilities;
  ACE_OS::memset (&device_capabilities, 0, sizeof (struct v4l2_capability));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_QUERYCAP,
                       &device_capabilities);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", returning\n"),
                fd_in, ACE_TEXT ("VIDIOC_QUERYCAP")));
    return;
  } // end IF
  std::ostringstream converter;
  converter << ((device_capabilities.version >> 16) & 0xFF)
            << '.'
            << ((device_capabilities.version >> 8) & 0xFF)
            << '.'
            << (device_capabilities.version & 0xFF);
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("device file descriptor: %d\n---------------------------\ndriver: \"%s\"\ncard: \"%s\"\nbus: \"%s\"\nversion: \"%s\"\ncapabilities: %u\ndevice capabilities: %u\nreserved: %u|%u|%u\n"),
              fd_in,
              ACE_TEXT (device_capabilities.driver),
              ACE_TEXT (device_capabilities.card),
              ACE_TEXT (device_capabilities.bus_info),
              ACE_TEXT (converter.str ().c_str ()),
              device_capabilities.capabilities,
              device_capabilities.device_caps,
              device_capabilities.reserved[0],device_capabilities.reserved[1],device_capabilities.reserved[2]));
}
bool
Stream_Module_Device_Tools::initializeCapture (int fd_in,
                                               v4l2_memory method_in,
                                               __u32& numberOfBuffers_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initializeCapture"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (Stream_Module_Device_Tools::canStream (fd_in));

  struct v4l2_requestbuffers request_buffers;
  ACE_OS::memset (&request_buffers, 0, sizeof (struct v4l2_requestbuffers));
  request_buffers.count = numberOfBuffers_inout;
  request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request_buffers.memory = method_in;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_REQBUFS,
                       &request_buffers);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  fd_in, ACE_TEXT ("VIDIOC_REQBUFS")));
      return false;
    } // end IF
    goto no_support;
  } // end IF
  numberOfBuffers_inout = request_buffers.count;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocated %d device buffer slots...\n"),
              numberOfBuffers_inout));

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("device (fd was: %d) does not support streaming method (was: %d), aborting\n"),
              fd_in, method_in));

  return false;
}
bool
Stream_Module_Device_Tools::initializeOverlay (int fd_in,
                                               const struct v4l2_window& window_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initializeOverlay"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (Stream_Module_Device_Tools::canOverlay (fd_in));

  // step1: set up frame-buffer (if necessary)
  struct v4l2_framebuffer framebuffer;
  ACE_OS::memset (&framebuffer, 0, sizeof (struct v4l2_framebuffer));
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_FBUF,
                       &framebuffer);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != EINVAL) // 22
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  fd_in, ACE_TEXT ("VIDIOC_G_FBUF")));
      return false;
    } // end IF
    goto no_support;
  } //IF end

  // *TODO*: configure frame-buffer options

  result = v4l2_ioctl (fd_in,
                       VIDIOC_S_FBUF,
                       &framebuffer);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_FBUF")));
    goto error;
  } //

  // step2: set up output format / overlay window
  struct v4l2_format format;
  ACE_OS::memset (&format, 0, sizeof (struct v4l2_format));

  format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
  format.fmt.win = window_in;
  // *TODO*: configure format options

  result = v4l2_ioctl (fd_in,
                       VIDIOC_S_FMT,
                       &format);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_FMT")));
    goto error;
  } //
  // *TODO*: verify that format now contains the requested configuration

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("device (was: %d) does not support overlays, aborting\n"),
              fd_in));
error:
  return false;
}

unsigned int
Stream_Module_Device_Tools::queued (int fd_in,
                                    unsigned int numberOfBuffers_in,
                                    unsigned int& done_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::queued"));

  unsigned int result = 0;

  // init return value(s)
  done_out = 0;

  int result_2 = -1;
  struct v4l2_buffer buffer;
  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for (unsigned int i = 0;
       i < numberOfBuffers_in;
       ++i)
  {
    buffer.index = i;
    result_2 = v4l2_ioctl (fd_in,
                           VIDIOC_QUERYBUF,
                           &buffer);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                  fd_in, ACE_TEXT ("VIDIOC_QUERYBUF")));

    if (buffer.flags & V4L2_BUF_FLAG_DONE)
      ++done_out;
    if (buffer.flags & V4L2_BUF_FLAG_QUEUED)
      ++result;
  } // end FOR

  return result;
}

bool
Stream_Module_Device_Tools::setCaptureFormat (int fd_in,
                                              const struct v4l2_format& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);
  ACE_ASSERT (format_in.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  int result = v4l2_ioctl (fd_in,
                           VIDIOC_S_FMT,
                           &format_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_FMT")));
    return false;
  } // end IF

  return true;
}
bool
Stream_Module_Device_Tools::getCaptureFormat (int fd_in,
                                              struct v4l2_format& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);

  int result = -1;
  ACE_OS::memset (&format_out, 0, sizeof (struct v4l2_format));
  format_out.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_FMT,
                       &format_out);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_FMT")));
    return false;
  } // end IF
//  ACE_ASSERT (format_out.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  return true;
}
bool
Stream_Module_Device_Tools::getFrameRate (int fd_in,
                                          struct v4l2_fract& frameRate_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getFrameRate"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);

  // initialize return value(s)
  ACE_OS::memset (&frameRate_out, 0, sizeof (struct v4l2_fract));

  int result = -1;
  struct v4l2_streamparm stream_parameters;
  ACE_OS::memset (&stream_parameters, 0, sizeof (struct v4l2_streamparm));
  stream_parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_PARM")));
    return false;
  } // end IF
  if ((stream_parameters.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) == 0)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the device driver does not support frame interval settings, continuing\n")));

  //  ACE_ASSERT (stream_parameters.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);

  frameRate_out = stream_parameters.parm.capture.timeperframe;

  return true;
}
bool
Stream_Module_Device_Tools::setFrameRate (int fd_in,
                                          const struct v4l2_fract& interval_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setFrameRate"));

  // sanity check(s)
  ACE_ASSERT (fd_in != -1);

  int result = -1;
  struct v4l2_streamparm stream_parameters;
  ACE_OS::memset (&stream_parameters, 0, sizeof (struct v4l2_streamparm));
  stream_parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  result = v4l2_ioctl (fd_in,
                       VIDIOC_G_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_G_PARM")));
    return false;
  } // end IF
//  ACE_ASSERT (stream_parameters.type == V4L2_BUF_TYPE_VIDEO_CAPTURE);
  // sanity check(s)
  if ((stream_parameters.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) == 0)
    goto no_support;

  stream_parameters.parm.capture.timeperframe = interval_in;

  result = v4l2_ioctl (fd_in,
                       VIDIOC_S_PARM,
                       &stream_parameters);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                fd_in, ACE_TEXT ("VIDIOC_S_PARM")));
    return false;
  } // end IF

  // validate setting
  if ((stream_parameters.parm.capture.timeperframe.numerator   != interval_in.numerator)  ||
      (stream_parameters.parm.capture.timeperframe.denominator != interval_in.denominator))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the device driver has not accepted the supplied frame rate (requested: %u, is: %u), continuing\n"),
                interval_in.denominator, stream_parameters.parm.capture.timeperframe.denominator));

  return true;

no_support:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("the device driver does not support frame interval settings, aborting\n")));
  return false;
}

std::string
Stream_Module_Device_Tools::formatToString (uint32_t format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::formatToString"));

  std::string result;

  return result;
}
#endif
