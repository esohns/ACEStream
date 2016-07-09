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
#include "dshow.h"
#include "dvdmedia.h"
#include "evr.h"
//#include "ksuuids.h"
#include "mfapi.h"
#include "mferror.h"
//#include "mftransform.h"
#include "mtype.h"
#include "oleauto.h"
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
  // DirectShow
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

  // Media Foundation
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Default, ACE_TEXT_ALWAYS_CHAR ("MF default")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Audio, ACE_TEXT_ALWAYS_CHAR ("MF audio")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Video, ACE_TEXT_ALWAYS_CHAR ("MF video")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Protected, ACE_TEXT_ALWAYS_CHAR ("MF protected")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_SAMI, ACE_TEXT_ALWAYS_CHAR ("MF SAMI")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Script, ACE_TEXT_ALWAYS_CHAR ("MF script")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Image, ACE_TEXT_ALWAYS_CHAR ("MF image")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_HTML, ACE_TEXT_ALWAYS_CHAR ("MF HTML")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Binary, ACE_TEXT_ALWAYS_CHAR ("MF binary")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_FileTransfer, ACE_TEXT_ALWAYS_CHAR ("MF file transfer")));
  Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Stream, ACE_TEXT_ALWAYS_CHAR ("MF stream")));

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

  // Media Foundation
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB32, ACE_TEXT_ALWAYS_CHAR ("RGB32")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_ARGB32, ACE_TEXT_ALWAYS_CHAR ("ARGB32")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB24, ACE_TEXT_ALWAYS_CHAR ("RGB24")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB555, ACE_TEXT_ALWAYS_CHAR ("RGB555")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB565, ACE_TEXT_ALWAYS_CHAR ("RGB565")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB8, ACE_TEXT_ALWAYS_CHAR ("RGB8")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_AI44, ACE_TEXT_ALWAYS_CHAR ("AI44")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_AYUV, ACE_TEXT_ALWAYS_CHAR ("AYUV")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YUY2, ACE_TEXT_ALWAYS_CHAR ("YUY2")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YVYU, ACE_TEXT_ALWAYS_CHAR ("YVYU")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YVU9, ACE_TEXT_ALWAYS_CHAR ("YVU9")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_UYVY, ACE_TEXT_ALWAYS_CHAR ("UYVY")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_NV11, ACE_TEXT_ALWAYS_CHAR ("NV11")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_NV12, ACE_TEXT_ALWAYS_CHAR ("NV12")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YV12, ACE_TEXT_ALWAYS_CHAR ("YV12")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_I420, ACE_TEXT_ALWAYS_CHAR ("I420")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_IYUV, ACE_TEXT_ALWAYS_CHAR ("IYUV")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y210, ACE_TEXT_ALWAYS_CHAR ("Y210")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y216, ACE_TEXT_ALWAYS_CHAR ("Y216")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y410, ACE_TEXT_ALWAYS_CHAR ("Y410")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y416, ACE_TEXT_ALWAYS_CHAR ("Y416")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y41P, ACE_TEXT_ALWAYS_CHAR ("Y41P")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y41T, ACE_TEXT_ALWAYS_CHAR ("Y41T")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y42T, ACE_TEXT_ALWAYS_CHAR ("Y42T")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P210, ACE_TEXT_ALWAYS_CHAR ("P210")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P216, ACE_TEXT_ALWAYS_CHAR ("P216")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P010, ACE_TEXT_ALWAYS_CHAR ("P010")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P016, ACE_TEXT_ALWAYS_CHAR ("P016")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v210, ACE_TEXT_ALWAYS_CHAR ("V210")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v216, ACE_TEXT_ALWAYS_CHAR ("V216")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v410, ACE_TEXT_ALWAYS_CHAR ("V410")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP43, ACE_TEXT_ALWAYS_CHAR ("MP43")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP4S, ACE_TEXT_ALWAYS_CHAR ("MP4S")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_M4S2, ACE_TEXT_ALWAYS_CHAR ("M4S2")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP4V, ACE_TEXT_ALWAYS_CHAR ("MP4V")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV1, ACE_TEXT_ALWAYS_CHAR ("WMV1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV2, ACE_TEXT_ALWAYS_CHAR ("WMV2")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV3, ACE_TEXT_ALWAYS_CHAR ("WMV3")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WVC1, ACE_TEXT_ALWAYS_CHAR ("WVC1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MSS1, ACE_TEXT_ALWAYS_CHAR ("MSS1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MSS2, ACE_TEXT_ALWAYS_CHAR ("MSS2")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MPG1, ACE_TEXT_ALWAYS_CHAR ("MPG1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVSL, ACE_TEXT_ALWAYS_CHAR ("DVSL")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVSD, ACE_TEXT_ALWAYS_CHAR ("DVSD")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVHD, ACE_TEXT_ALWAYS_CHAR ("DVHD")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DV25, ACE_TEXT_ALWAYS_CHAR ("DV25")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DV50, ACE_TEXT_ALWAYS_CHAR ("DV50")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVH1, ACE_TEXT_ALWAYS_CHAR ("DVH1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVC,  ACE_TEXT_ALWAYS_CHAR ("DVC")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H264, ACE_TEXT_ALWAYS_CHAR ("H264")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MJPG, ACE_TEXT_ALWAYS_CHAR ("MJPG")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_420O, ACE_TEXT_ALWAYS_CHAR ("420O")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_HEVC, ACE_TEXT_ALWAYS_CHAR ("HEVC")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_HEVC_ES, ACE_TEXT_ALWAYS_CHAR ("HEVC_ES")));
#if (WINVER >= _WIN32_WINNT_WIN8)
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H263, ACE_TEXT_ALWAYS_CHAR ("H263")));
#endif
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H264_ES, ACE_TEXT_ALWAYS_CHAR ("H264_ES")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MPEG2, ACE_TEXT_ALWAYS_CHAR ("MPEG2")));

  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Float, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_DTS, ACE_TEXT_ALWAYS_CHAR ("DTS")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3_SPDIF")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_DRM, ACE_TEXT_ALWAYS_CHAR ("DRM")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudioV8, ACE_TEXT_ALWAYS_CHAR ("WMAudioV8")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudioV9, ACE_TEXT_ALWAYS_CHAR ("WMAudioV9")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudio_Lossless, ACE_TEXT_ALWAYS_CHAR ("WMAudio_Lossless")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMASPDIF, ACE_TEXT_ALWAYS_CHAR ("WMASPDIF")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MSP1, ACE_TEXT_ALWAYS_CHAR ("MSP1")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MP3, ACE_TEXT_ALWAYS_CHAR ("MP3")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MPEG, ACE_TEXT_ALWAYS_CHAR ("MPEG")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AAC, ACE_TEXT_ALWAYS_CHAR ("AAC")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_ADTS, ACE_TEXT_ALWAYS_CHAR ("ADTS")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_NB, ACE_TEXT_ALWAYS_CHAR ("AMR_NB")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("AMR_WB")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_WP, ACE_TEXT_ALWAYS_CHAR ("AMR_WP")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3")));
  Stream_Module_Device_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_DDPlus, ACE_TEXT_ALWAYS_CHAR ("Dolby_DDPlus")));

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
//void
//Stream_Module_Device_Tools::dump (IMFSourceReader* IMFSourceReader_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::dump"));
//
//  // sanity check(s)
//  ACE_ASSERT (IMFSourceReader_in);
//
//  HRESULT result = S_OK;
//  DWORD count = 0;
//  IMFMediaType* media_type_p = NULL;
//  while (true)
//  {
//    media_type_p = NULL;
//    result =
//      IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                              count,
//                                              &media_type_p);
//    if (FAILED (result)) break;
//
//    ACE_DEBUG ((LM_INFO,
//                ACE_TEXT ("#%d: %s\n"),
//                count + 1,
//                ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (media_type_p).c_str ())));
//
//    // clean up
//    media_type_p->Release ();
//
//    ++count;
//  } // end WHILE
//  if (result != MF_E_NO_MORE_TYPES)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", returning\n"),
//                count,
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return;
//  } // end IF
//}
bool
Stream_Module_Device_Tools::expand (TOPOLOGY_PATH_T& path_inout,
                                    TOPOLOGY_PATHS_T& paths_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::expand"));

  TOPOLOGY_PATH_ITERATOR_T iterator = path_inout.begin ();
  for (;
       iterator != path_inout.end ();
       ++iterator);
  --iterator;

  HRESULT result = E_FAIL;
  DWORD number_of_outputs = 0;
  result = (*iterator)->GetOutputCount (&number_of_outputs);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_outputs == 0)
    return false; // done (no changes)

  IMFTopologyNode* topology_node_p = NULL;
  DWORD input_index = 0;
  TOPOLOGY_PATH_T topology_path;
  for (DWORD i = 0;
       i < number_of_outputs;
       ++i)
  {
    topology_node_p = NULL;
    result = (*iterator)->GetOutput (i,
                                     &topology_node_p,
                                     &input_index);
    ACE_ASSERT (SUCCEEDED (result));

    topology_path = path_inout;
    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = topology_path.begin ();
         iterator_2 != topology_path.end ();
         ++iterator_2)
      (*iterator_2)->AddRef ();
    topology_path.push_back (topology_node_p);
    paths_inout.push_back (topology_path);
  } // end FOR

  return true;
}
void
Stream_Module_Device_Tools::dump (IMFTopology* IMFTopology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  HRESULT result = S_OK;
  WORD count = 0;
  result = IMFTopology_in->GetNodeCount (&count);
  ACE_ASSERT (SUCCEEDED (result));
  TOPOID id = 0;
  result = IMFTopology_in->GetTopologyID (&id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("topology (id: %q, %u nodes)\n"),
              id,
              count));
  IMFTopologyNode* topology_node_p = NULL;
  MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  //for (WORD i = 0;
  //     i < count;
  //     ++i)
  //{
  //  topology_node_p = NULL;
  //  result = IMFTopology_in->GetNode (i,
  //                                    &topology_node_p);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  result = topology_node_p->GetNodeType (&node_type);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  result = topology_node_p->GetTopoNodeID (&id);
  //  ACE_ASSERT (SUCCEEDED (result));

  //  ACE_DEBUG ((LM_INFO,
  //              ACE_TEXT ("#%u: %s (id: %q)\n"),
  //              i + 1,
  //              ACE_TEXT (Stream_Module_Device_Tools::nodeTypeToString (node_type).c_str ()),
  //              id));

  //  topology_node_p->Release ();
  //} // end FOR
  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("topology contains no source nodes, done\n")));

    // clean up
    collection_p->Release ();

    return;
  } // end IF
  TOPOLOGY_PATHS_T topology_paths;
  TOPOLOGY_PATH_T topology_path;
  IUnknown* unknown_p = NULL;
  for (DWORD i = 0;
       i < number_of_source_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    topology_node_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();
      collection_p->Release ();

      return;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_p);
    topology_path.clear ();
    topology_path.push_back (topology_node_p);
    topology_paths.push_back (topology_path);
  } // end FOR
  collection_p->Release ();

  bool changed = false;
  do
  {
    changed = false;
    for (TOPOLOGY_PATHS_ITERATOR_T iterator = topology_paths.begin ();
         iterator != topology_paths.end ();
         ++iterator)
    {
      if (Stream_Module_Device_Tools::expand (*iterator,
                                              topology_paths))
      {
        for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
             iterator_2 != (*iterator).end ();
             ++iterator_2)
          (*iterator_2)->Release ();
        topology_paths.erase (iterator);
        changed = true;
        break;
      } // end IF
    } // end FOR
    if (!changed)
      break;
  } while (true);

  std::string topology_string_base, topology_string;
  std::ostringstream converter;
  int index = 0;
  for (TOPOLOGY_PATHS_ITERATOR_T iterator = topology_paths.begin ();
       iterator != topology_paths.end ();
       ++iterator, ++index)
  {
    topology_string_base = ACE_TEXT_ALWAYS_CHAR ("#");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << index + 1;
    topology_string_base += converter.str ();
    topology_string_base += ACE_TEXT_ALWAYS_CHAR (": ");

    TOPOLOGY_PATH_ITERATOR_T iterator_3;
    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
         iterator_2 != (*iterator).end ();
         ++iterator_2)
    {
      result = (*iterator_2)->GetNodeType (&node_type);
      ACE_ASSERT (SUCCEEDED (result));
      result = (*iterator_2)->GetTopoNodeID (&id);
      ACE_ASSERT (SUCCEEDED (result));

      topology_string = Stream_Module_Device_Tools::nodeTypeToString (node_type);
      topology_string += ACE_TEXT_ALWAYS_CHAR (" (");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << id;
      topology_string += converter.str ();
      topology_string += ACE_TEXT_ALWAYS_CHAR (")");

      iterator_3 = iterator_2;
      if (++iterator_3 != (*iterator).end ())
        topology_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

      topology_string_base += topology_string;

      // clean up
      (*iterator_2)->Release ();
    } // end FOR
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s\n"),
                ACE_TEXT (topology_string_base.c_str ())));
  } // end FOR
}
void
Stream_Module_Device_Tools::dump (IMFTransform* IMFTransform_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);

  HRESULT result = S_OK;
  DWORD number_of_input_streams = 0;
  DWORD number_of_output_streams = 0;
  result =
    IMFTransform_in->GetStreamCount (&number_of_input_streams,
                                     &number_of_output_streams);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetStreamCount(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  DWORD* input_stream_ids_p = NULL;
  ACE_NEW_NORETURN (input_stream_ids_p,
                    DWORD [number_of_input_streams]);
  DWORD* output_stream_ids_p = NULL;
  ACE_NEW_NORETURN (output_stream_ids_p,
                    DWORD [number_of_output_streams]);
  if (!input_stream_ids_p || !output_stream_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));

    // clean up
    if (input_stream_ids_p)
      delete [] input_stream_ids_p;
    if (output_stream_ids_p)
      delete[] output_stream_ids_p;

    return;
  } // end IF
  result =
    IMFTransform_in->GetStreamIDs (number_of_input_streams,
                                   input_stream_ids_p,
                                   number_of_output_streams,
                                   output_stream_ids_p);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetStreamIDs(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      delete [] input_stream_ids_p;
      delete [] output_stream_ids_p;

      return;
    } // end IF

    int i = 0;
    for (;
         i < static_cast<int> (number_of_input_streams);
         ++i)
      input_stream_ids_p[i] = i;
    for (i = 0;
         i < static_cast<int> (number_of_output_streams);
         ++i)
      output_stream_ids_p[i] = i;
  } // end IF
  delete [] input_stream_ids_p;
  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  for (int i = 0;
       i < static_cast<int> (number_of_output_streams);
       ++i)
  {
    count = 0;

    while (true)
    {
      media_type_p = NULL;
      result =
        IMFTransform_in->GetOutputAvailableType (output_stream_ids_p[i],
                                                 count,
                                                 &media_type_p);
      if (FAILED (result)) break; // MF_E_TRANSFORM_TYPE_NOT_SET: 0xC00D6D60L

      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("#%d: %s\n"),
                  count + 1,
                  ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (media_type_p).c_str ())));

      // clean up
      media_type_p->Release ();

      ++count;
    } // end WHILE
  } // end FOR
  delete [] output_stream_ids_p;
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetOutputAvailableType(%d): \"%s\", returning\n"),
                count,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
}

bool
Stream_Module_Device_Tools::isCompressed (const struct _GUID& subType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::isCompressed"));

  // *TODO*: this is probably incomplete
  return (!Stream_Module_Device_Tools::isChromaLuminance (subType_in) &&
          !Stream_Module_Device_Tools::isRGB (subType_in));

}
bool
Stream_Module_Device_Tools::isRGB (const struct _GUID& subType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::isRGB"));

  return ((subType_in == MFVideoFormat_RGB32)  ||
          (subType_in == MFVideoFormat_ARGB32) ||
          (subType_in == MFVideoFormat_RGB24)  ||
          (subType_in == MFVideoFormat_RGB555) ||
          (subType_in == MFVideoFormat_RGB565) ||
          (subType_in == MFVideoFormat_RGB8));
}
bool
Stream_Module_Device_Tools::isChromaLuminance (const struct _GUID& subType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::isChromaLuminance"));

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
}

IPin*
Stream_Module_Device_Tools::pin (IBaseFilter* filter_in,
                                 enum _PinDirection direction_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::pin"));

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

  //IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
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

      return NULL;
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
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("0x%@ [\"%s\"]: no %s pin found, aborting\n"),
    //            filter_in,
    //            ACE_TEXT (Stream_Module_Device_Tools::name (filter_in).c_str ()),
    //            ((direction_in == PINDIR_INPUT) ? ACE_TEXT ("input") : ACE_TEXT ("output"))));
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

//bool
//Stream_Module_Device_Tools::getSourceReader (IMFMediaSource*& mediaSource_inout,
//                                             WCHAR*& symbolicLink_out,
//                                             UINT32& symbolicLinkSize_out,
//                                             const IDirect3DDeviceManager9* IDirect3DDeviceManager9_in,
//                                             const IMFSourceReaderCallback* callback_in,
//                                             bool isChromaLuminance_in, 
//                                             IMFSourceReaderEx*& sourceReader_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getSourceReader"));
//
//  bool result = false;
//
//  if (symbolicLinkSize_out)
//  {
//    // sanity check(s)
//    ACE_ASSERT (symbolicLink_out);
//
//    CoTaskMemFree (symbolicLink_out);
//    symbolicLink_out = NULL;
//    symbolicLinkSize_out = 0;
//  } // end IF
//  if (sourceReader_out)
//  {
//    sourceReader_out->Release ();
//    sourceReader_out = NULL;
//  } // end IF
//
//  if (!mediaSource_inout)
//    if (!Stream_Module_Device_Tools::getMediaSource (std::string (),
//                                                     mediaSource_inout,
//                                                     symbolicLink_out,
//                                                     symbolicLinkSize_out))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), aborting\n")));
//      return false;
//    } // end IF
//  ACE_ASSERT (mediaSource_inout);
//
//  IMFAttributes* attributes_p = NULL;
//  HRESULT result_2 = MFCreateAttributes (&attributes_p, 10);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    return false;
//  } // end IF
//
//  IUnknown* iunknown_p = NULL;
//  //result_2 = attributes_p->SetUINT32 (MF_READWRITE_DISABLE_CONVERTERS,
//  //                                    FALSE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_READWRITE_DISABLE_CONVERTERS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  if (IDirect3DDeviceManager9_in)
//  {
//    result_2 = attributes_p->SetUINT32 (MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS,
//                                        TRUE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  if (callback_in)
//  {
//    iunknown_p = const_cast<IMFSourceReaderCallback*> (callback_in);
//    result_2 =
//      attributes_p->SetUnknown (MF_SOURCE_READER_ASYNC_CALLBACK,
//                                iunknown_p);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUnknown(): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  if (IDirect3DDeviceManager9_in)
//  {
//    iunknown_p =
//      const_cast<IDirect3DDeviceManager9*> (IDirect3DDeviceManager9_in);
//    result_2 =
//      attributes_p->SetUnknown (MF_SOURCE_READER_D3D_MANAGER,
//                                iunknown_p);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUnknown(MF_SOURCE_READER_D3D_MANAGER): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//
//    result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISABLE_DXVA,
//                                        FALSE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISABLE_DXVA): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  //result_2 = attributes_p->SetUnknown (MF_SOURCE_READER_MEDIASOURCE_CONFIG,
//  //                                     NULL); // IPropertyBag handle
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_MEDIASOURCE_CONFIG): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS,
//  //                                    TRUE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  // *NOTE*: allow conversion from YUV to RGB-32 ?
//  // *NOTE*: "...Avoid this setting if you are using Direct3D to display the
//  //         video frames, because the GPU generally provides better video
//  //         processing capabilities.
//  //         If this attribute is TRUE, the following attributes must be FALSE :
//  //         • MF_SOURCE_READER_D3D_MANAGER
//  //         • MF_READWRITE_DISABLE_CONVERTERS ..."
//  if (!IDirect3DDeviceManager9_in)
//  {
//    if (isChromaLuminance_in)
//    {
//      result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING,
//                                          TRUE);
//      if (FAILED (result_2))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING): \"%s\", aborting\n"),
//                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//        goto error;
//      } // end IF
//    } // end IF
//  } // end IF
//  else
//  {
//    // *NOTE*: "... If this attribute is TRUE, the
//    //         MF_READWRITE_DISABLE_CONVERTERS attribute must be FALSE. ..."
//    result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING,
//                                        TRUE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISABLE_CAMERA_PLUGINS,
//                                      TRUE);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISABLE_CAMERA_PLUGINS): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN,
//  //                                    TRUE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS,
//  //                                    FALSE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  //result_2 = attributes_p->SetUnknown (MFT_FIELDOFUSE_UNLOCK_Attribute,
//  //                                     NULL); // IMFFieldOfUseMFTUnlock handle
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUnknown(MFT_FIELDOFUSE_UNLOCK_Attribute): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  IMFSourceReader* source_reader_p = NULL;
//  result_2 = MFCreateSourceReaderFromMediaSource (mediaSource_inout,
//                                                  attributes_p,
//                                                  &source_reader_p);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateSourceReaderFromMediaSource(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//  result_2 = source_reader_p->QueryInterface (IID_PPV_ARGS (&sourceReader_out));
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::QueryInterface(IID_IMFSourceReaderEx): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//
//    // clean up
//    source_reader_p->Release ();
//
//    goto error;
//  } // end IF
//  source_reader_p->Release ();
//
//  result = true;
//
//error:
//  if (attributes_p)
//    attributes_p->Release ();
//
//  return result;
//}
bool
Stream_Module_Device_Tools::getMediaSource (const std::string& deviceName_in,
                                            IMFMediaSource*& mediaSource_out,
                                            WCHAR*& symbolicLink_out,
                                            UINT32& symbolicLinkSize_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getDeviceHandle"));

  bool result = false;

  if (mediaSource_out)
  {
    mediaSource_out->Release ();
    mediaSource_out = NULL;
  } // end IF
  if (symbolicLinkSize_out)
  {
    // sanity check(s)
    ACE_ASSERT (symbolicLink_out);

    CoTaskMemFree (symbolicLink_out);
    symbolicLink_out = NULL;
    symbolicLinkSize_out = 0;
  } // end IF

  IMFAttributes* attributes_p = NULL;
  HRESULT result_2 = MFCreateAttributes (&attributes_p, 1);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return false;
  } // end IF

  result_2 =
    attributes_p->SetGUID (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                           MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::SetGUID(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF

  IMFActivate** devices_pp = NULL;
  UINT32 count = 0;
  result_2 = MFEnumDeviceSources (attributes_p,
                                  &devices_pp,
                                  &count);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (devices_pp);
  if (count == 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no capture devices found, aborting\n")));
    goto error;
  } // end IF

  unsigned int index = 0;
  if (!deviceName_in.empty ())
  {
    WCHAR friendly_name_string[BUFSIZ];
    UINT32 length;
    bool found = false;
    for (UINT32 i = 0; i < count; i++)
    {
      ACE_OS::memset (friendly_name_string, 0, sizeof (friendly_name_string));
      length = 0;
      result_2 =
        devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                      friendly_name_string,
                                      sizeof (friendly_name_string),
                                      &length);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      if (ACE_OS::strcmp (friendly_name_string,
                          ACE_TEXT_ALWAYS_WCHAR (deviceName_in.c_str ())) == 0)
      {
        found = true;
        index = i;
        break;
      } // end IF
    } // end FOR
    if (!found)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("capture device (was: \"%s\") not found, aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      goto error;
    } // end IF
  } // end IF
  result_2 =
    devices_pp[index]->ActivateObject (IID_PPV_ARGS (&mediaSource_out));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF

  result_2 =
    devices_pp[index]->GetAllocatedString (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
                                           &symbolicLink_out,
                                           &symbolicLinkSize_out);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF

  result = true;

error:
  if (attributes_p)
    attributes_p->Release ();

  for (UINT32 i = 0; i < count; i++)
    devices_pp[i]->Release ();
  CoTaskMemFree (devices_pp);

  if (!result && mediaSource_out)
  {
    mediaSource_out->Release ();
    mediaSource_out = NULL;
  } // end IF
  if (!result && symbolicLink_out)
  {
    CoTaskMemFree (symbolicLink_out);
    symbolicLink_out = NULL;
  } // end IF
  if (!result)
    symbolicLinkSize_out = 0;

  return result;
}
bool
Stream_Module_Device_Tools::getMediaSource (const IMFMediaSession* IMFMediaSession_in,
                                            IMFMediaSource*& IMFMediaSource_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getMediaSource"));

  // initialize return value(s)
  if (IMFMediaSource_out)
  {
    IMFMediaSource_out->Release ();
    IMFMediaSource_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IMFMediaSession_in);

  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFTopology* topology_p = NULL;
  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
  //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
  //         --> (try to) wait for the next MESessionTopologySet event
  // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
  //         still fails with MF_E_INVALIDREQUEST)
  HRESULT result = E_FAIL;
  do
  {
    result =
      const_cast<IMFMediaSession*> (IMFMediaSession_in)->GetFullTopology (flags,
                                                                          0,
                                                                          &topology_p);
  } while (result == MF_E_INVALIDREQUEST);
  if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  if (!Stream_Module_Device_Tools::getMediaSource (topology_p,
                                                   IMFMediaSource_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), aborting\n")));

    // clean up
    topology_p->Release ();

    return false;
  } // end IF
  topology_p->Release ();
  ACE_ASSERT (IMFMediaSource_out);

  return true;
}
bool
Stream_Module_Device_Tools::getMediaSource (const IMFTopology* IMFTopology_in,
                                            IMFMediaSource*& IMFMediaSource_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getMediaSource"));

  // initialize return value(s)
  if (IMFMediaSource_out)
  {
    IMFMediaSource_out->Release ();
    IMFMediaSource_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  IMFCollection* collection_p = NULL;
  HRESULT result =
    const_cast<IMFTopology*> (IMFTopology_in)->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("media session topology has no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();

    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();

    return false;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;
  result = topology_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&IMFMediaSource_out));
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release ();

  return true;
}

bool
Stream_Module_Device_Tools::getDirect3DDevice (const HWND windowHandle_in,
                                               const IMFMediaType* IMFMediaType_in,
                                               IDirect3DDevice9Ex*& IDirect3DDevice9Ex_out,
                                               struct _D3DPRESENT_PARAMETERS_& presentationParameters_out,
                                               IDirect3DDeviceManager9*& IDirect3DDeviceManager9_out,
                                               UINT& resetToken_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getDirect3DDevice"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  if (IDirect3DDevice9Ex_out)
  {
    IDirect3DDevice9Ex_out->Release ();
    IDirect3DDevice9Ex_out = NULL;
  } // end IF
  ACE_OS::memset (&presentationParameters_out,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  ACE_ASSERT (resetToken_out == 0);

  UINT32 width, height;
  result = MFGetAttributeSize (const_cast<IMFMediaType*> (IMFMediaType_in),
                               MF_MT_FRAME_SIZE,
                               &width, &height);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  IDirect3D9Ex* Direct3D9_p = NULL;
  result = Direct3DCreate9Ex (D3D_SDK_VERSION, &Direct3D9_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Direct3DCreate9Ex(%d): \"%s\", aborting\n"),
                D3D_SDK_VERSION,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  struct _D3DDISPLAYMODE d3d_display_mode;
  ACE_OS::memset (&d3d_display_mode,
                  0,
                  sizeof (struct _D3DDISPLAYMODE));
  result = Direct3D9_p->GetAdapterDisplayMode (D3DADAPTER_DEFAULT,
                                               &d3d_display_mode);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::GetAdapterDisplayMode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = Direct3D9_p->CheckDeviceType (D3DADAPTER_DEFAULT,
                                         D3DDEVTYPE_HAL,
                                         d3d_display_mode.Format,
                                         D3DFMT_X8R8G8B8,
                                         TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::CheckDeviceType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  //if (Mode == "fullscreen")
  //{
  //  presentationParameters_out.BackBufferWidth = x;
  //  presentationParameters_out.BackBufferHeight = y;
  //} // end IF
  //else if (Mode == "winmode")
  //{
  presentationParameters_out.BackBufferWidth = width;
  presentationParameters_out.BackBufferHeight = height;
  //} // end IF
  presentationParameters_out.BackBufferFormat = D3DFMT_X8R8G8B8;
  presentationParameters_out.BackBufferCount =
    MODULE_DEV_CAM_MEDIAFOUNDATION_DEFAULT_BACK_BUFFERS;
  //presentationParameters_out.MultiSampleType = ;
  //presentationParameters_out.MultiSampleQuality = ;
  presentationParameters_out.SwapEffect = D3DSWAPEFFECT_FLIP;
  presentationParameters_out.hDeviceWindow = windowHandle_in;
  presentationParameters_out.Windowed = true;
  //presentationParameters_out.EnableAutoDepthStencil = ;
  //presentationParameters_out.AutoDepthStencilFormat = ;
  presentationParameters_out.Flags =
    (D3DPRESENTFLAG_LOCKABLE_BACKBUFFER            |
     //D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL           |
     D3DPRESENTFLAG_DEVICECLIP                     |
     D3DPRESENTFLAG_VIDEO);//                          |
     //D3DPRESENTFLAG_NOAUTOROTATE                   |
     //D3DPRESENTFLAG_UNPRUNEDMODE                   |
     //D3DPRESENTFLAG_OVERLAY_LIMITEDRGB             |
     //D3DPRESENTFLAG_OVERLAY_YCbCr_BT709            |
     //D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC            |
     //D3DPRESENTFLAG_RESTRICTED_CONTENT             |
     //D3DPRESENTFLAG_RESTRICT_SHARED_RESOURCE_DRIVER);
  //d3d_present_parameters.FullScreen_RefreshRateInHz = ;
  presentationParameters_out.PresentationInterval =
    D3DPRESENT_INTERVAL_IMMEDIATE;
  DWORD behavior_flags = (//D3DCREATE_ADAPTERGROUP_DEVICE          |
                          //D3DCREATE_DISABLE_DRIVER_MANAGEMENT    |
                          //D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX |
                          //D3DCREATE_DISABLE_PRINTSCREEN          |
                          //D3DCREATE_DISABLE_PSGP_THREADING       |
                          //D3DCREATE_ENABLE_PRESENTSTATS          |
                          D3DCREATE_FPU_PRESERVE                 |
                          D3DCREATE_HARDWARE_VERTEXPROCESSING    |
                          //D3DCREATE_MIXED_VERTEXPROCESSING       |
                          D3DCREATE_MULTITHREADED);//                |
                          //D3DCREATE_NOWINDOWCHANGES              |
                          //D3DCREATE_PUREDEVICE                   |
                          //D3DCREATE_SCREENSAVER                  |
                          //D3DCREATE_SOFTWARE_VERTEXPROCESSING);
  result =
    Direct3D9_p->CreateDeviceEx (D3DADAPTER_DEFAULT,          // adapter
                                 D3DDEVTYPE_HAL,              // device type
                                 windowHandle_in,             // focus window handle
                                 behavior_flags,              // behavior flags
                                 &presentationParameters_out, // presentation parameters
                                 NULL,                        // (fullscreen) display mode
                                 &IDirect3DDevice9Ex_out);    // return value: device handle
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3D9Ex::CreateDeviceEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  Direct3D9_p->Release ();
  Direct3D9_p = NULL;

  result = DXVA2CreateDirect3DDeviceManager9 (&resetToken_out,
                                              &IDirect3DDeviceManager9_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DXVA2CreateDirect3DDeviceManager9(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result =
    IDirect3DDeviceManager9_out->ResetDevice (IDirect3DDevice9Ex_out,
                                              resetToken_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDeviceManager9::ResetDevice(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (Direct3D9_p)
    Direct3D9_p->Release ();
  if (IDirect3DDevice9Ex_out)
  {
    IDirect3DDevice9Ex_out->Release ();
    IDirect3DDevice9Ex_out = NULL;
  } // end IF
  ACE_OS::memset (&presentationParameters_out,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  resetToken_out = 0;

  return false;

continue_:
  return true;
}
bool
Stream_Module_Device_Tools::initializeDirect3DManager (const IDirect3DDevice9Ex* IDirect3DDevice9Ex_in,
                                                       IDirect3DDeviceManager9*& IDirect3DDeviceManager9_out,
                                                       UINT& resetToken_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::initializeDirect3DManager"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  ACE_ASSERT (resetToken_out == 0);

  // sanity check(s)
  ACE_ASSERT (IDirect3DDevice9Ex_in);

  result =
    DXVA2CreateDirect3DDeviceManager9 (&resetToken_out,
                                       &IDirect3DDeviceManager9_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DXVA2CreateDirect3DDeviceManager9(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result =
    IDirect3DDeviceManager9_out->ResetDevice (const_cast<IDirect3DDevice9Ex*> (IDirect3DDevice9Ex_in),
                                              resetToken_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDeviceManager9::ResetDevice(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (IDirect3DDeviceManager9_out)
  {
    IDirect3DDeviceManager9_out->Release ();
    IDirect3DDeviceManager9_out = NULL;
  } // end IF
  resetToken_out = 0;

  return false;

continue_:
  return true;
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
Stream_Module_Device_Tools::loadDeviceTopology (const std::string& deviceName_in,
                                                IMFMediaSource*& IMFMediaSource_inout,
                                                const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::loadDeviceTopology"));

  // initialize return value(s)
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

  TOPOID node_id = 0;
  HRESULT result = MFCreateTopology (&IMFTopology_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                       MFTOPOLOGY_DXVA_FULL);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                       FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                       MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                       FALSE);
  ACE_ASSERT (SUCCEEDED (result));

  IMFTopologyNode* topology_node_p = NULL;
  result = MFCreateTopologyNode (MF_TOPOLOGY_SOURCESTREAM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  WCHAR* symbolic_link_p = NULL;
  UINT32 symbolic_link_size = 0;
  if (!IMFMediaSource_inout)
  {
    if (!Stream_Module_Device_Tools::getMediaSource (deviceName_in,
                                                     IMFMediaSource_inout,
                                                     symbolic_link_p,
                                                     symbolic_link_size))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      goto error;
    } // end IF
    CoTaskMemFree (symbolic_link_p);
  } // end IF
  ACE_ASSERT (IMFMediaSource_inout);
  result = topology_node_p->SetUnknown (MF_TOPONODE_SOURCE,
                                        IMFMediaSource_inout);
  ACE_ASSERT (SUCCEEDED (result));
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  result =
    IMFMediaSource_inout->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result =
    topology_node_p->SetUnknown (MF_TOPONODE_PRESENTATION_DESCRIPTOR,
                                 presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  result = topology_node_p->SetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                        stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_descriptor_p->Release ();

  result = IMFTopology_out->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added source node (id: %q)...\n"),
              node_id));

  // *NOTE*: add 'dummy' sink node so the topology can be loaded ?
  if (!IMFSampleGrabberSinkCallback2_in)
    goto continue_;

  IMFMediaType* media_type_p = NULL;
  if (!Stream_Module_Device_Tools::getCaptureFormat (IMFMediaSource_inout,
                                                     media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
  IMFActivate* activate_p = NULL;
  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,
                                       const_cast<IMFSampleGrabberSinkCallback2*> (IMFSampleGrabberSinkCallback2_in),
                                       &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    media_type_p->Release ();

    goto error;
  } // end IF
  media_type_p->Release ();
  // To run as fast as possible, set this attribute (requires Windows 7):
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));

  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  IMFStreamSink* stream_sink_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();

  IMFTopologyNode* topology_node_2 = NULL;
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_2->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_2->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_2->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_2->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->AddNode (topology_node_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_2->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added 'dummy' sink node (id: %q)...\n"),
              node_id));
  result = topology_node_p->ConnectOutput (0,
                                           topology_node_2,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_2->Release ();
continue_:
  topology_node_p->Release ();

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
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

  OLECHAR GUID_string[CHARS_IN_GUID];
  ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
  int count = 0;

  // convert RGB / YUV / MJPEG / ... --> RGB ?
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
    ACE_ASSERT (count == CHARS_IN_GUID);

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
    ACE_ASSERT (count == CHARS_IN_GUID);

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
    ACE_ASSERT (count == CHARS_IN_GUID);

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
Stream_Module_Device_Tools::enableDirectXAcceleration (IMFTopology* IMFTopology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::enableDirectXAcceleration"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  // step1: find a(n output) node that supports the Direct3D manager (typically:
  //        EVR)
  IDirect3DDeviceManager9* direct3D_device_manager_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = IMFTopology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_output_nodes = 0;
  result = collection_p->GetElementCount (&number_of_output_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  TOPOID node_id = 0;
  for (DWORD i = 0;
       i < number_of_output_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    topology_node_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();
      collection_p->Release ();

      return false;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_p);

    result = MFGetService (topology_node_p,
                           MR_VIDEO_ACCELERATION_SERVICE,
                           IID_PPV_ARGS (&direct3D_device_manager_p));
    if (SUCCEEDED (result))
    {
      result = topology_node_p->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("node (id was: %q) supports MR_VIDEO_ACCELERATION_SERVICE...\n"),
                  node_id));
      break;
    } // end IF

    topology_node_p->Release ();
  } // end FOR
  collection_p->Release ();
  topology_node_p->Release ();
  if (!direct3D_device_manager_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no topology node supports MR_VIDEO_ACCELERATION_SERVICE, aborting\n")));
    return false;
  } // end IF

  // step2: iterate over all transfrom nodes and activate DirectX acceleration
  WORD count = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  IMFTransform* transform_p = NULL;
  IMFAttributes* attributes_p = NULL;
  UINT32 is_Direct3D_aware = 0;
  ULONG_PTR pointer_p = reinterpret_cast<ULONG_PTR> (direct3D_device_manager_p);
  result = IMFTopology_in->GetNodeCount (&count);
  ACE_ASSERT (SUCCEEDED (result));
  for (WORD i = 0;
       i < count;
       ++i)
  {
    topology_node_p = NULL;
    result = IMFTopology_in->GetNode (i, &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->GetNodeType (&node_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_type != MF_TOPOLOGY_TRANSFORM_NODE)
    {
      topology_node_p->Release ();
      continue;
    } // end IF

    unknown_p = NULL;
    result = topology_node_p->GetObject (&unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    transform_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();
      topology_node_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();

    result = transform_p->GetAttributes (&attributes_p);
    ACE_ASSERT (SUCCEEDED (result));
    is_Direct3D_aware = MFGetAttributeUINT32 (attributes_p,
                                              MF_SA_D3D_AWARE,
                                              FALSE);
    if (!is_Direct3D_aware)
    {
      // clean up
      attributes_p->Release ();
      transform_p->Release ();
      topology_node_p->Release ();

      continue;
    } // end IF
    attributes_p->Release ();

    result = transform_p->ProcessMessage (MFT_MESSAGE_SET_D3D_MANAGER,
                                          pointer_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();
      topology_node_p->Release ();

      goto error;
    } // end IF
    transform_p->Release ();

    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE, TRUE);
    ACE_ASSERT (SUCCEEDED (result));

    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("node (id was: %q) enabled MR_VIDEO_ACCELERATION_SERVICE...\n"),
                node_id));

    topology_node_p->Release ();
  } // end FOR
  direct3D_device_manager_p->Release ();

  return true;

error:
  if (direct3D_device_manager_p)
    direct3D_device_manager_p->Release ();

  return false;
}
bool
Stream_Module_Device_Tools::addGrabber (const IMFMediaType* IMFMediaType_in,
                                        const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                        IMFTopology* IMFTopology_in,
                                        TOPOID& grabberNodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::addGrabber"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaType_in);
  ACE_ASSERT (IMFSampleGrabberSinkCallback2_in);
  ACE_ASSERT (IMFTopology_in);

  // initialize return value(s)
  grabberNodeId_out = 0;

  // step1: create sample grabber sink
  IMFActivate* activate_p = NULL;
  HRESULT result =
    MFCreateSampleGrabberSinkActivate (const_cast<IMFMediaType*> (IMFMediaType_in),
                                       const_cast<IMFSampleGrabberSinkCallback2*> (IMFSampleGrabberSinkCallback2_in),
                                       &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    activate_p->Release ();

    return false;
  } // end IF
  activate_p->Release ();

  // step2: add node to topology
  IMFTopologyNode* topology_node_p = NULL;
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  IMFStreamSink* stream_sink_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_in->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&grabberNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added grabber node (id: %q)...\n"),
              grabberNodeId_out));
  topology_node_p->Release ();
  topology_node_p = NULL;

  if (!Stream_Module_Device_Tools::append (IMFTopology_in,
                                           grabberNodeId_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::append(%q), aborting\n"),
                grabberNodeId_out));
    goto error;
  } // end IF

  return true;

error:
  if (media_sink_p)
    media_sink_p->Release ();
  if (grabberNodeId_out)
  {
    if (topology_node_p)
    {
      topology_node_p->Release ();
      topology_node_p = NULL;
    } // end IF
    result = IMFTopology_in->GetNodeByID (grabberNodeId_out,
                                          &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_in->RemoveNode (topology_node_p);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::RemoveNode(%q): \"%s\", continuing\n"),
                  grabberNodeId_out,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    grabberNodeId_out = 0;
  } // end IF
  if (topology_node_p)
    topology_node_p->Release ();

  return false;
}
bool
Stream_Module_Device_Tools::addRenderer (const HWND windowHandle_in,
                                         IMFTopology* IMFTopology_in,
                                         TOPOID& rendererNodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::addRenderer"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  // initialize return value(s)
  rendererNodeId_out = 0;

  // step1: create (EVR) renderer
  IMFActivate* activate_p = NULL;
  HRESULT result = MFCreateVideoRendererActivate (windowHandle_in,
                                                  &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  activate_p->Release ();

  //  return false;
  //} // end IF
  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    activate_p->Release ();

    return false;
  } // end IF
  activate_p->Release ();

  //result = MFCreateVideoRenderer (IID_PPV_ARGS (&media_sink_p));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoRenderer(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  IMFPresentationClock* presentation_clock_p = NULL;
  result = MFCreatePresentationClock (&presentation_clock_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreatePresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  IMFPresentationTimeSource* presentation_time_source_p = NULL;
  result = MFCreateSystemTimeSource (&presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSystemTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  result = presentation_clock_p->SetTimeSource (presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::SetTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_time_source_p->Release ();
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  presentation_time_source_p->Release ();
  result = media_sink_p->SetPresentationClock (presentation_clock_p);
  if (FAILED (result)) // MF_E_NOT_INITIALIZED: 0xC00D36B6L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::SetPresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  result = presentation_clock_p->Start (0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::Start(0): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  presentation_clock_p->Release ();

  //IMFTransform* transform_p = NULL;
  //result =
  //  MFCreateVideoMixer (NULL,                         // owner
  //                      IID_IDirect3DDevice9,         // device
  //                      IID_PPV_ARGS (&transform_p)); // return value: interface handle
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  // step2: add node to topology
  IMFTopologyNode* topology_node_p = NULL;
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  IMFStreamSink* stream_sink_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_in->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
              rendererNodeId_out));
  topology_node_p->Release ();
  topology_node_p = NULL;

  if (!Stream_Module_Device_Tools::append (IMFTopology_in,
                                           rendererNodeId_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::append(%q), aborting\n"),
                rendererNodeId_out));
    goto error;
  } // end IF

  if (!Stream_Module_Device_Tools::enableDirectXAcceleration (IMFTopology_in))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::enableDirectXAcceleration(), continuing\n")));

  return true;

error:
  if (media_sink_p)
    media_sink_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (rendererNodeId_out)
  {
    topology_node_p = NULL;
    result = IMFTopology_in->GetNodeByID (rendererNodeId_out,
                                          &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_in->RemoveNode (topology_node_p);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::RemoveNode(%q): \"%s\", continuing\n"),
                  rendererNodeId_out,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    // *TODO*: apparently, topology_node_p is invalid after RemoveNode()
    //topology_node_p->Release ();
    rendererNodeId_out = 0;
  } // end IF

  return false;
}
bool
Stream_Module_Device_Tools::loadRendererTopology (const std::string& deviceName_in,
                                                  const IMFMediaType* IMFMediaType_in,
                                                  const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                  const HWND windowHandle_in,
                                                  TOPOID& sampleGrabberSinkNodeId_out,
                                                  TOPOID& rendererNodeId_out,
                                                  IMFTopology*& IMFTopology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::loadRendererTopology"));

  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
  IMFMediaSource* media_source_p = NULL;
  IMFMediaType* media_type_p = NULL;
  IMFActivate* activate_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  std::string module_string;

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;
  if (!IMFTopology_inout)
  {
    if (!Stream_Module_Device_Tools::loadDeviceTopology (deviceName_in,
                                                         media_source_p,
                                                         NULL, // do not load a dummy sink
                                                         IMFTopology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      goto error;
    } // end IF
    release_topology = true;
  } // end IF
  else if (!Stream_Module_Device_Tools::getMediaSource (IMFTopology_inout,
                                                        media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), aborting\n")));
    goto error;
  } // end ELSE IF
  ACE_ASSERT (media_source_p);

  // step1: retrieve source node
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = IMFTopology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();
    collection_p = NULL;

    goto error;
  } // end IF
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();
    unknown_p = NULL;

    goto error;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;

  // step1a: set default capture media type ?
  UINT32 item_count = 0;
  result = const_cast<IMFMediaType*> (IMFMediaType_in)->GetCount (&item_count);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (!item_count)
  {
    if (!Stream_Module_Device_Tools::getCaptureFormat (media_source_p,
                                                       media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default/preset capture format...\n")));
  } // end IF
  else if (!Stream_Module_Device_Tools::copyMediaType (IMFMediaType_in,
                                                       media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::copyMediaType(), aborting\n")));
    goto error;
  } // end IF
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  // step2: add decoder nodes ?
  //BOOL is_compressed = false;
  //result = media_type_p->IsCompressedFormat (&is_compressed);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFMediaType::IsCompressedFormat(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  if (!Stream_Module_Device_Tools::isCompressed (sub_type))
    goto continue_;

  IMFActivate** decoders_p = NULL;
  UINT32 number_of_decoders = 0;
  mft_register_type_info.guidMajorType = MFMediaType_Video;
  UINT32 flags = (MFT_ENUM_FLAG_SYNCMFT        |
                  MFT_ENUM_FLAG_ASYNCMFT       |
                  MFT_ENUM_FLAG_HARDWARE       |
                  MFT_ENUM_FLAG_FIELDOFUSE     |
                  MFT_ENUM_FLAG_LOCALMFT       |
                  MFT_ENUM_FLAG_TRANSCODE_ONLY |
                  MFT_ENUM_FLAG_SORTANDFILTER);
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
  //if (!media_source_p)
  //{
  //  result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
  //                                      IID_PPV_ARGS (&media_source_p));
  //  ACE_ASSERT (SUCCEEDED (result));
  //} // end IF
  //if (!Stream_Module_Device_Tools::getCaptureFormat (media_source_p,
  //                                                   media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //media_source_p->Release ();
  //media_source_p = NULL;

  //IMFAttributes* attributes_p = NULL;
  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;

    result = MFTEnumEx (MFT_CATEGORY_VIDEO_DECODER, // category
                        flags,                      // flags
                        &mft_register_type_info,    // input type
                        NULL,                       // output type
                        &decoders_p,                // array of decoders
                        &number_of_decoders);       // size of array
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    if (number_of_decoders <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ())));
      goto error;
    } // end IF

    module_string = Stream_Module_Device_Tools::activateToString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));
    for (UINT32 i = 0; i < number_of_decoders; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p);
    //result = transform_p->GetAttributes (&attributes_p);
    //ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    //ACE_ASSERT (SUCCEEDED (result));
    //attributes_p->Release ();
    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF

    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added transform node (id: %q)...\n"),
    //            node_id));
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release ();
    media_type_p = NULL;
    if (!Stream_Module_Device_Tools::getOutputFormat (transform_p,
                                                      media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = transform_p->SetOutputType (0,
                                         media_type_p,
                                         0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    transform_p->Release ();
    transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%q: added decoder for \"%s\": \"%s\"...\n"),
                node_id,
                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ()),
                ACE_TEXT (module_string.c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (!Stream_Module_Device_Tools::isCompressed (sub_type))
      break; // done
  } // end WHILE

  // transform to RGB ?
  if (Stream_Module_Device_Tools::isRGB (sub_type))
    goto continue_;

  mft_register_type_info.guidSubtype = sub_type;

  decoders_p = NULL;
  number_of_decoders = 0;
  result = MFTEnumEx (MFT_CATEGORY_VIDEO_PROCESSOR, // category
                      flags,                        // flags
                      &mft_register_type_info,      // input type
                      NULL,                         // output type
                      &decoders_p,                  // array of decoders
                      &number_of_decoders);         // size of array
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (number_of_decoders <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot find processor for: \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ())));
    goto error;
  } // end IF

  module_string = Stream_Module_Device_Tools::activateToString (decoders_p[0]);

  result = decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
  ACE_ASSERT (SUCCEEDED (result));
  for (UINT32 i = 0; i < number_of_decoders; i++)
    decoders_p[i]->Release ();
  CoTaskMemFree (decoders_p);
  result = transform_p->SetInputType (0,
                                      media_type_p,
                                      0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF

  result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
  result = topology_node_p->SetObject (transform_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added transform node (id: %q)...\n"),
  //            node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

  int i = 0;
  while (!Stream_Module_Device_Tools::isRGB (sub_type))
  {
    media_type_p->Release ();
    media_type_p = NULL;
    result = transform_p->GetOutputAvailableType (0,
                                                  i,
                                                  &media_type_p);

    ACE_ASSERT (SUCCEEDED (result));
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    ++i;
  } // end WHILE
  //result = media_type_p->DeleteAllItems ();
  //ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (Stream_Module_Device_Tools::copyAttribute (IMFMediaType_in,
                                                         media_type_p,
                                                         MF_MT_FRAME_RATE));
  ACE_ASSERT (Stream_Module_Device_Tools::copyAttribute (IMFMediaType_in,
                                                         media_type_p,
                                                         MF_MT_FRAME_SIZE));
  ACE_ASSERT (Stream_Module_Device_Tools::copyAttribute (IMFMediaType_in,
                                                         media_type_p,
                                                         MF_MT_INTERLACE_MODE));
  ACE_ASSERT (Stream_Module_Device_Tools::copyAttribute (IMFMediaType_in,
                                                         media_type_p,
                                                         MF_MT_PIXEL_ASPECT_RATIO));
  result = transform_p->SetOutputType (0,
                                       media_type_p,
                                       0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
#if defined (_DEBUG)
  media_type_p->Release ();
  media_type_p = NULL;
  result = transform_p->GetOutputCurrentType (0,
                                              &media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("output format: \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (media_type_p).c_str ())));
#endif
  IMFVideoProcessorControl* video_processor_control_p = NULL;
  result =
    transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
  ACE_ASSERT (SUCCEEDED (result));
  transform_p->Release ();
  transform_p = NULL;
  // *TODO*: (for some unknown reason,) this does nothing...
  result = video_processor_control_p->SetMirror (MIRROR_VERTICAL);
  //result = video_processor_control_p->SetRotation (ROTATION_NORMAL);
  ACE_ASSERT (SUCCEEDED (result));
  video_processor_control_p->Release ();

  // debug info
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added processor for \"%s\": \"%s\"...\n"),
              node_id,
              ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype).c_str ()),
              ACE_TEXT (module_string.c_str ())));

continue_:
  // step3: add tee node ?
  if ((!IMFSampleGrabberSinkCallback2_in && !windowHandle_in) ||
      ((IMFSampleGrabberSinkCallback2_in && !windowHandle_in) || // XOR
      (!IMFSampleGrabberSinkCallback2_in &&  windowHandle_in)))
    goto continue_2;

  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added tee node (id: %q)...\n"),
              node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

continue_2:
  // step4: add sample grabber sink ?
  if (!IMFSampleGrabberSinkCallback2_in)
    goto continue_3;

  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,
                                       const_cast<IMFSampleGrabberSinkCallback2*> (IMFSampleGrabberSinkCallback2_in),
                                       &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  // To run as fast as possible, set this attribute (requires Windows 7):
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));

  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  //result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->SetCurrentMediaType (media_type_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&sampleGrabberSinkNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added sample grabber sink node (id: %q)...\n"),
              sampleGrabberSinkNodeId_out));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;

continue_3:
  // step5: add video renderer sink ?
  if (!windowHandle_in)
    goto continue_4;

  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  media_type_handler_p = NULL;
  result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->SetCurrentMediaType (media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
              rendererNodeId_out));
  result =
    source_node_p->ConnectOutput ((IMFSampleGrabberSinkCallback2_in ? 1 : 0),
                                  topology_node_p,
                                  0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;
  media_source_p->Release ();
  media_source_p = NULL;
  media_type_p->Release ();
  media_type_p = NULL;

  if (!Stream_Module_Device_Tools::enableDirectXAcceleration (IMFTopology_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::enableDirectXAcceleration(), aborting\n")));
    goto error;
  } // end IF

continue_4:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();
  if (release_topology)
  {
    IMFTopology_inout->Release ();
    IMFTopology_inout = NULL;
  } // end IF

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
  OLECHAR GUID_string[CHARS_IN_GUID];
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
      ACE_ASSERT (nCount == CHARS_IN_GUID);

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
      ACE_ASSERT (nCount == CHARS_IN_GUID);

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
//bool
//Stream_Module_Device_Tools::loadTargetRendererTopology (const IMFMediaType* IMFMediaType_in,
//                                                        const IMFSampleGrabberSinkCallback* IMFSampleGrabberSinkCallback_in,
//                                                        const HWND windowHandle_in,
//                                                        IMFTopology*& IMFTopology_inout)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::loadTargetRendererTopology"));
//
//  bool release_topology = false;
//  struct _GUID sub_type = { 0 };
//  MFT_REGISTER_TYPE_INFO mft_register_type_info = { 0 };
//
//  // initialize return value(s)
//  IMFMediaSource* media_source_p = NULL;
//
//  if (!IMFTopology_inout)
//  {
//    if (!Stream_Module_Device_Tools::loadDeviceTopology (deviceName_in,
//                                                         media_source_p,
//                                                         IMFTopology_inout))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
//                  ACE_TEXT (deviceName_in.c_str ())));
//      goto error;
//    } // end IF
//    release_topology = true;
//  } // end IF
//
//  // step1: retrieve source node
//  IMFTopologyNode* source_node_p = NULL;
//  IMFCollection* collection_p = NULL;
//  HRESULT result =
//    IMFTopology_inout->GetSourceNodeCollection (&collection_p);
//  ACE_ASSERT (SUCCEEDED (result));
//  DWORD number_of_source_nodes = 0;
//  result = collection_p->GetElementCount (&number_of_source_nodes);
//  ACE_ASSERT (SUCCEEDED (result));
//  if (number_of_source_nodes <= 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("topology contains no source nodes, aborting\n")));
//
//    // clean up
//    collection_p->Release ();
//    collection_p = NULL;
//
//    goto error;
//  } // end IF
//  IUnknown* unknown_p = NULL;
//  result = collection_p->GetElement (0, &unknown_p);
//  ACE_ASSERT (SUCCEEDED (result));
//  collection_p->Release ();
//  collection_p = NULL;
//  ACE_ASSERT (unknown_p);
//  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//    // clean up
//    unknown_p->Release ();
//    unknown_p = NULL;
//
//    goto error;
//  } // end IF
//  unknown_p->Release ();
//  unknown_p = NULL;
//
//  // step2: add decoder nodes ?
//  IMFActivate** decoders_p = NULL;
//  UINT32 number_of_decoders = 0;
//  mft_register_type_info.guidMajorType = MFMediaType_Video;
//  UINT32 flags = (MFT_ENUM_FLAG_SYNCMFT |
//    MFT_ENUM_FLAG_ASYNCMFT |
//    MFT_ENUM_FLAG_HARDWARE |
//    MFT_ENUM_FLAG_FIELDOFUSE |
//    MFT_ENUM_FLAG_LOCALMFT |
//    MFT_ENUM_FLAG_TRANSCODE_ONLY |
//    MFT_ENUM_FLAG_SORTANDFILTER);
//  IMFTransform* transform_p = NULL;
//  IMFTopologyNode* topology_node_p = NULL;
//  if (!media_source_p)
//  {
//    result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
//      IID_PPV_ARGS (&media_source_p));
//    ACE_ASSERT (SUCCEEDED (result));
//  } // end IF
//  IMFMediaType* media_type_p = NULL;
//  if (!Stream_Module_Device_Tools::getCaptureFormat (media_source_p,
//    media_type_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
//    goto error;
//  } // end IF
//  media_source_p->Release ();
//  media_source_p = NULL;
//
//  if (!Stream_Module_Device_Tools::isCompressed (media_type_p))
//    goto continue_;
//
//  while (true)
//  {
//    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &sub_type);
//    ACE_ASSERT (SUCCEEDED (result));
//    mft_register_type_info.guidSubtype = sub_type;
//
//    result = MFTEnumEx (MFT_CATEGORY_VIDEO_DECODER, // category
//      flags,                      // flags
//      &mft_register_type_info,    // input type
//      NULL,                       // output type
//      &decoders_p,                // array of decoders
//      &number_of_decoders);       // size of array
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//        ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
//        ACE_TEXT (Common_Tools::error2String (result).c_str ()),
//        ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ())));
//      goto error;
//    } // end IF
//    if (number_of_decoders <= 0)
//    {
//      ACE_DEBUG ((LM_ERROR,
//        ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
//        ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ())));
//      goto error;
//    } // end IF
//
//    result =
//      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
//    ACE_ASSERT (SUCCEEDED (result));
//    for (UINT32 i = 0; i < number_of_decoders; i++)
//      decoders_p[i]->Release ();
//    CoTaskMemFree (decoders_p);
//    result = transform_p->SetInputType (0,
//      media_type_p,
//      0);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//        ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
//        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      transform_p->Release ();
//
//      goto error;
//    } // end IF
//
//    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
//      &topology_node_p);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//        ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
//        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      transform_p->Release ();
//
//      goto error;
//    } // end IF
//    result = topology_node_p->SetObject (transform_p);
//    ACE_ASSERT (SUCCEEDED (result));
//    result = IMFTopology_inout->AddNode (topology_node_p);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//        ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
//        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//      goto error;
//    } // end IF
//    result = source_node_p->ConnectOutput (0,
//      topology_node_p,
//      0);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//        ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
//        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      transform_p->Release ();
//
//      goto error;
//    } // end IF
//    source_node_p->Release ();
//    source_node_p = topology_node_p;
//    topology_node_p = NULL;
//
//    media_type_p->Release ();
//    media_type_p = NULL;
//    //if (!Stream_Module_Device_Tools::getOutputFormat (transform_p,
//    //                                                  media_type_p))
//    //{
//    //  ACE_DEBUG ((LM_ERROR,
//    //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(): \"%s\", aborting\n"),
//    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//    //  // clean up
//    //  transform_p->Release ();
//
//    //  goto error;
//    //} // end IF
//    result = transform_p->GetOutputCurrentType (0,
//      &media_type_p);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//        ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
//        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      transform_p->Release ();
//
//      goto error;
//    } // end IF
//    transform_p->Release ();
//    transform_p = NULL;
//
//    // debug info
//    ACE_DEBUG ((LM_DEBUG,
//      ACE_TEXT ("added decoder for \"%s\"...\n"),
//      ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ())));
//
//    if (!Stream_Module_Device_Tools::isCompressed (media_type_p))
//      break; // done
//  } // end WHILE
//  media_type_p->Release ();
//  media_type_p = NULL;
//
//continue_:
//  // step3: add tee node ?
//  if (!IMFSampleGrabberSinkCallback_in && !windowHandle_in)
//    goto continue_2;
//
//  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
//    &topology_node_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  result = IMFTopology_inout->AddNode (topology_node_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  result = source_node_p->ConnectOutput (0,
//    topology_node_p,
//    0);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  source_node_p->Release ();
//  source_node_p = topology_node_p;
//  topology_node_p = NULL;
//
//continue_2:
//  // step4: add sample grabber sink ?
//  IMFActivate* activate_p = NULL;
//
//  if (!IMFSampleGrabberSinkCallback_in)
//    goto continue_3;
//
//  result =
//    MFCreateSampleGrabberSinkActivate (const_cast<IMFMediaType*> (IMFMediaType_in),
//      const_cast<IMFSampleGrabberSinkCallback*> (IMFSampleGrabberSinkCallback_in),
//      &activate_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//    // To run as fast as possible, set this attribute (requires Windows 7):
//  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
//    TRUE);
//  ACE_ASSERT (SUCCEEDED (result));
//  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
//    &topology_node_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  result = topology_node_p->SetObject (activate_p);
//  ACE_ASSERT (SUCCEEDED (result));
//  activate_p->Release ();
//  activate_p = NULL;
//  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
//  ACE_ASSERT (SUCCEEDED (result));
//  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
//  ACE_ASSERT (SUCCEEDED (result));
//  result = IMFTopology_inout->AddNode (topology_node_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  result = source_node_p->ConnectOutput (0,
//    topology_node_p,
//    0);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  topology_node_p->Release ();
//  topology_node_p = NULL;
//
//continue_3:
//  // step5: add video renderer sink ?
//  if (!windowHandle_in)
//    goto continue_4;
//
//  result = MFCreateVideoRendererActivate (windowHandle_in,
//    &activate_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//    //// *NOTE*: select a (custom) video presenter
//    //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
//    //                              );
//    //if (FAILED (result))
//    //{
//    //  ACE_DEBUG ((LM_ERROR,
//    //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
//    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    //  goto error;
//    //} // end IF
//
//  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
//    &topology_node_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  result = topology_node_p->SetObject (activate_p);
//  ACE_ASSERT (SUCCEEDED (result));
//  activate_p->Release ();
//  activate_p = NULL;
//  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
//  ACE_ASSERT (SUCCEEDED (result));
//  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
//  ACE_ASSERT (SUCCEEDED (result));
//  result = IMFTopology_inout->AddNode (topology_node_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  result =
//    source_node_p->ConnectOutput ((IMFSampleGrabberSinkCallback_in ? 1 : 0),
//      topology_node_p,
//      0);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
//      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  topology_node_p->Release ();
//  topology_node_p = NULL;
//
//continue_4:
//  return true;
//
//error:
//  if (media_source_p)
//    media_source_p->Release ();
//  if (media_type_p)
//    media_type_p->Release ();
//  if (source_node_p)
//    source_node_p->Release ();
//  if (topology_node_p)
//    topology_node_p->Release ();
//  if (activate_p)
//    activate_p->Release ();
//  if (release_topology)
//  {
//    IMFTopology_inout->Release ();
//    IMFTopology_inout = NULL;
//  } // end IF
//
//  return false;
//}
bool
Stream_Module_Device_Tools::setTopology (IMFTopology* IMFTopology_in,
                                         IMFMediaSession*& IMFMediaSession_inout,
                                         bool waitForCompletion_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setTopology"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  HRESULT result = E_FAIL;
  bool release_media_session = false;
  IMFTopoLoader* topology_loader_p = NULL;
  IMFTopology* topology_p = NULL;

  // initialize return value(s)
  if (!IMFMediaSession_inout)
  {
    IMFAttributes* attributes_p = NULL;
    result = MFCreateAttributes (&attributes_p, 4);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
    ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
    //ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = MFCreateMediaSession (attributes_p,
                                   &IMFMediaSession_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      attributes_p->Release ();

      goto error;
    } // end IF
    attributes_p->Release ();
    release_media_session = true;
  } // end IF
  ACE_ASSERT (IMFMediaSession_inout);

  result = MFCreateTopoLoader (&topology_loader_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopoLoader(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_loader_p->Load (IMFTopology_in,
                                    &topology_p,
                                    NULL);
  if (FAILED (result)) // MF_E_INVALIDMEDIATYPE    : 0xC00D36B4L
  {                    // MF_E_NO_MORE_TYPES       : 0xC00D36B9L
                       // MF_E_TOPO_CODEC_NOT_FOUND: 0xC00D5212L
                       // MF_E_TOPO_UNSUPPORTED:     0xC00D5214L
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopoLoader::Load(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    Stream_Module_Device_Tools::dump (topology_p);
    goto error;
  } // end IF
  topology_loader_p->Release ();
  topology_loader_p = NULL;

  DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);// |
                          //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                          //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
  result = IMFMediaSession_inout->SetTopology (topology_flags,
                                               topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_p->Release ();
  topology_p = NULL;

  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous, so subsequent calls
  //         to retrieve the topology handle will fail (MF_E_INVALIDREQUEST)
  //         --> wait a little ?
  if (!waitForCompletion_in)
    goto continue_;

  IMFMediaEvent* media_event_p = NULL;
  bool received_topology_set_event = false;
  MediaEventType event_type = MEUnknown;
  do
  {
    media_event_p = NULL;
    result = IMFMediaSession_inout->GetEvent (0,
                                              &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologySet)
      received_topology_set_event = true;
    media_event_p->Release ();
  } while (!received_topology_set_event);

continue_:
  return true;

error:
  if (topology_loader_p)
    topology_loader_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (release_media_session)
  { 
    IMFMediaSession_inout->Release ();
    IMFMediaSession_inout = NULL;
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
  //IAMStreamConfig* stream_config_p = NULL;
  IPin* pin_p = Stream_Module_Device_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::name (filter_p).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  filter_p->Release ();
  //result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IPin::QueryInterface(IAMStreamConfig): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  pin_p->Release ();

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

  //  return false;
  //} // end IF
  //stream_config_p->Release ();
  IPin* pin_2 = NULL;
  //struct _PinInfo pin_info;
  //ACE_OS::memset (&pin_info, 0, sizeof (struct _PinInfo));
  std::list<std::wstring>::const_iterator iterator_2;
  IPin* pin_3 = NULL;
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

    pin_2 = Stream_Module_Device_Tools::pin (filter_p, PINDIR_INPUT);
    if (!pin_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: has no input pin, aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (filter_p).c_str ())));

      // clean up
      filter_p->Release ();

      return false;
    } // end IF
    //result = pin_2->QueryPinInfo (&pin_info);
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IPin::QueryPinInfo(): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    //  // clean up
    //  pin_2->Release ();
    //  pin_p->Release ();

    //  return false;
    //} // end IF

    iterator_2 = iterator;
    --iterator_2;
   
    // pin already connected ? --> continue
    pin_3 = NULL;
    result = pin_p->ConnectedTo (&pin_3);
    if (SUCCEEDED (result))
    {
      IBaseFilter* filter_2 = Stream_Module_Device_Tools::pin2Filter (pin_p);
      ACE_ASSERT (filter_2);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: already connected, continuing\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (filter_2).c_str ())));

      // clean up
      pin_3->Release ();
      filter_2->Release ();

      goto continue_2;
    } // end IF

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
      pin_p->Release ();
      filter_p->Release ();

      return false;
    } // end IF
continue_:
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connected \"%s\" to \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));
continue_2:
    pin_2->Release ();
    pin_2 = NULL;
    pin_p->Release ();

    iterator_2 = iterator;
    if (++iterator_2 != graph_in.end ())
    {
      pin_p = Stream_Module_Device_Tools::pin (filter_p, PINDIR_OUTPUT);
      if (!pin_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: has no output pin, aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (filter_p).c_str ())));

        // clean up
        filter_p->Release ();

        break;
      } // end IF
    } // end IF
  } // end FOR

  return true;
}
bool
Stream_Module_Device_Tools::connectFirst (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::connectFirst"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

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
  IPin* pin_p = Stream_Module_Device_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::pin(PINDIR_OUTPUT), aborting\n")));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF

  IPin* pin_2 = NULL;
loop:
  result = pin_p->ConnectedTo (&pin_2);
  if (FAILED (result))
  {
    filter_p = Stream_Module_Device_Tools::pin2Filter (pin_p);
    ACE_ASSERT (filter_p);
    result = builder_in->Render (pin_p);
    if (FAILED (result))
    {

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::Render(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (filter_p).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      pin_p->Release ();

      return false;
    } // end IF

    return true;
  } // end IF
  ACE_ASSERT (pin_2);
  pin_p->Release ();

  filter_p = Stream_Module_Device_Tools::pin2Filter (pin_2);
  if (!filter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::pin2Filter(0x%@), aborting\n"),
                pin_2));

    // clean up
    pin_2->Release ();

    return false;
  } // end IF
  pin_2->Release ();
  pin_2 = NULL;

  pin_p = Stream_Module_Device_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    // clean up
    filter_p->Release ();

    return true; // filter has no output pin --> sink
  } // end IF
  filter_p->Release ();

  goto loop;

  ACE_NOTREACHED (return false;)
}
bool
Stream_Module_Device_Tools::connected (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::connected"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

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
  IPin* pin_p = Stream_Module_Device_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::pin(PINDIR_OUTPUT), aborting\n")));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF

  IPin* pin_2 = NULL;
loop:
  result = pin_p->ConnectedTo (&pin_2);
  if (FAILED (result))
  {
    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (pin_2);
  pin_p->Release ();

  filter_p = Stream_Module_Device_Tools::pin2Filter (pin_2);
  if (!filter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::pin2Filter(0x%@), aborting\n"),
                pin_2));

    // clean up
    pin_2->Release ();

    return false;
  } // end IF
  pin_2->Release ();
  pin_2 = NULL;

  pin_p = Stream_Module_Device_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    // clean up
    filter_p->Release ();

    return true; // filter has no output pin --> sink
  } // end IF
  filter_p->Release ();

  goto loop;

  ACE_NOTREACHED (return false;)
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
  //IAMStreamConfig* stream_config_p = NULL;
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
Stream_Module_Device_Tools::append (IMFTopology* IMFTopology_in,
                                    TOPOID nodeId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::append"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);
  ACE_ASSERT (nodeId_in);

  // step0: retrieve node handle
  bool add_tee_node = true;
  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  result = IMFTopology_in->GetNodeByID (nodeId_in,
                                        &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                nodeId_in,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);

  // step1: find a suitable upstream node
  IMFTopologyNode* topology_node_2 = NULL; // source/output node
  IMFTopologyNode* topology_node_3 = NULL; // upstream node
  IMFMediaType* media_type_p = NULL;
  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_nodes = 0;
  result = collection_p->GetElementCount (&number_of_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  if (number_of_nodes <= 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology contains no output nodes, continuing\n")));
use_source_node:
    add_tee_node = false;
    collection_p->Release ();
    collection_p = NULL;
    result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = collection_p->GetElementCount (&number_of_nodes);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_nodes <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("topology contains no source nodes, aborting\n")));

      // clean up
      collection_p->Release ();

      goto error;
    } // end IF
    result = collection_p->GetElement (0, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    collection_p->Release ();
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_2);

    DWORD input_index = 0;
    do
    {
      result = topology_node_2->GetOutputCount (&number_of_nodes);
      ACE_ASSERT (SUCCEEDED (result));
      if (number_of_nodes <= 0) break;

      topology_node_3 = NULL;
      result = topology_node_2->GetOutput (0,
                                           &topology_node_3,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      topology_node_2->Release ();
      topology_node_2 = topology_node_3;
    } while (true);

    goto continue_;
  } // end IF

  TOPOID node_id = 0;
  for (DWORD i = 0;
       i < number_of_nodes;
       ++i)
  {
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_2);

    result = topology_node_2->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_id == nodeId_in)
    {
      topology_node_2->Release ();
      topology_node_2 = NULL;
      continue;
    } // end IF

    break;
  } // end FOR
  if (!topology_node_2)
    goto use_source_node;
  collection_p->Release ();

  DWORD number_of_inputs = 0;
  result = topology_node_2->GetInputCount (&number_of_inputs);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (number_of_inputs > 0);
  DWORD output_index = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  result = topology_node_2->GetInput (0,
                                      &topology_node_3,
                                      &output_index);
  ACE_ASSERT (SUCCEEDED (result));

continue_:
  ACE_ASSERT (topology_node_p);
  ACE_ASSERT (topology_node_2);
  ACE_ASSERT (topology_node_3);

  result = topology_node_3->GetNodeType (&node_type);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type)
  {
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_3->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result));
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result));
      stream_descriptor_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&media_type_p);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release ();

      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      unknown_p = NULL;
      result = topology_node_3->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        unknown_p->Release ();
        topology_node_2->Release ();
        topology_node_3->Release ();

        goto error;
      } // end IF
      unknown_p->Release ();
      result = transform_p->GetOutputCurrentType (0,
                                                  &media_type_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        transform_p->Release ();
        topology_node_2->Release ();
        topology_node_3->Release ();

        goto error;
      } // end IF
      transform_p->Release ();

      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_3->GetOutputPrefType (0,
                                                   &media_type_p);
      ACE_ASSERT (SUCCEEDED (result));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type));

      // clean up
      topology_node_2->Release ();
      topology_node_3->Release ();

      goto error;
    }
  } // end SWITCH

  // step2: add a tee node ?
  IMFTopologyNode* topology_node_4 = NULL;
  if (!add_tee_node)
  {
    topology_node_4 = topology_node_3;
    topology_node_3 = NULL;
    goto continue_2;
  } // end IF

  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_3->Release ();

    goto error;
  } // end IF
  result = IMFTopology_in->AddNode (topology_node_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_3->Release ();
    topology_node_4->Release ();

    goto error;
  } // end IF
  node_id = 0;
  result = topology_node_4->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added tee node (id: %q)...\n"),
              node_id));

  // step3: connect the upstream node to the tee
  result = topology_node_3->ConnectOutput (0,
                                           topology_node_4,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_3->Release ();
    topology_node_4->Release ();

    goto error;
  } // end IF
  topology_node_3->Release ();
  result = topology_node_4->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (0,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (1,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  // step4: connect the (two) outputs
  result = topology_node_4->ConnectOutput (0,
                                           topology_node_2,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_4->Release ();

    goto error;
  } // end IF
  result = topology_node_2->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_2->Release ();

continue_2:
  result = topology_node_4->ConnectOutput ((add_tee_node ? 1 : 0),
                                           topology_node_p,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_4->Release ();

    goto error;
  } // end IF
  result = topology_node_p->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_p->Release ();
  topology_node_p->Release ();
  topology_node_4->Release ();

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();

  return false;
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
Stream_Module_Device_Tools::clear (IMFMediaSession* IMFMediaSession_in,
                                   bool waitForCompletion_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::clear"));

  // *NOTE*: this method is asynchronous (wait for MESessionTopologiesCleared)
  HRESULT result = IMFMediaSession_in->ClearTopologies ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::ClearTopologies(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  DWORD topology_flags = MFSESSION_SETTOPOLOGY_CLEAR_CURRENT;
  result = IMFMediaSession_in->SetTopology (topology_flags,
                                            NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(MFSESSION_SETTOPOLOGY_CLEAR_CURRENT): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
  //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
  //         --> (try to) wait for the next MESessionTopologySet event
  // *TODO*: this procedure doesn't always work as expected
  if (!waitForCompletion_in)
    return true;

  IMFMediaEvent* media_event_p = NULL;
  bool received_topology_set_event = false;
  MediaEventType event_type = MEUnknown;
  do
  {
    media_event_p = NULL;
    result = IMFMediaSession_in->GetEvent (0,
                                           &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologySet)
      received_topology_set_event = true;
    media_event_p->Release ();
  } while (!received_topology_set_event);

  return true;
}
bool
Stream_Module_Device_Tools::clear (IMFTopology* IMFTopology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::clear"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  HRESULT result = E_FAIL;
  IMFCollection* collection_p = NULL;
  result =
    IMFTopology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    // clean up
    collection_p->Release ();

    return true;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  DWORD number_of_outputs = 0;
  DWORD input_index = 0;
  IMFTopologyNode* topology_node_2 = NULL;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  TOPOID node_id = 0;
  DWORD number_of_outputs_2 = 0;
  IMFTopologyNode* topology_node_3 = NULL;
  for (DWORD i = 0;
       i < number_of_source_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      collection_p->Release ();
      unknown_p->Release ();

      return false;
    } // end IF
    unknown_p->Release ();

    number_of_outputs = 0;
    result = topology_node_p->GetOutputCount (&number_of_outputs);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_outputs <= 0)
    {
      // clean up
      topology_node_p->Release ();

      continue;
    } // end IF
    for (DWORD j = 0;
         j < number_of_outputs;
         ++j)
    {
      topology_node_2 = NULL;
      result = topology_node_p->GetOutput (j,
                                           &topology_node_2,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      result = topology_node_2->GetNodeType (&node_type);
      ACE_ASSERT (SUCCEEDED (result));
      if (node_type != MF_TOPOLOGY_TRANSFORM_NODE)
      {
        // clean up
        topology_node_p->Release ();
        topology_node_2->Release ();

        continue;
      } // end IF

      number_of_outputs_2 = 0;
      result = topology_node_2->GetOutputCount (&number_of_outputs_2);
      ACE_ASSERT (SUCCEEDED (result));
      for (DWORD k = 0;
           k < number_of_outputs_2;
           ++k)
      {
        topology_node_3 = NULL;
        result = topology_node_2->GetOutput (k,
                                             &topology_node_3,
                                             &input_index);
        ACE_ASSERT (SUCCEEDED (result));
        result = topology_node_3->GetTopoNodeID (&node_id);
        ACE_ASSERT (SUCCEEDED (result));
        result = IMFTopology_in->RemoveNode (topology_node_3);
        if (FAILED (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFTopology::RemoveNode(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));

          // clean up
          topology_node_3->Release ();
          topology_node_2->Release ();
          topology_node_p->Release ();
          collection_p->Release ();

          return false;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("removed node (id was: %q)...\n"),
                    node_id));
        topology_node_3->Release ();
      } // end FOR
      result = topology_node_2->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      result = IMFTopology_in->RemoveNode (topology_node_2);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopology::RemoveNode(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        topology_node_2->Release ();
        topology_node_p->Release ();
        collection_p->Release ();

        return false;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("removed transform node (id was: %q)...\n"),
                  node_id));
      topology_node_2->Release ();
    } // end FOR
    topology_node_p->Release ();
  } // end FOR
  collection_p->Release ();

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
Stream_Module_Device_Tools::disconnect (IMFTopologyNode* IMFTopologyNode_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (IMFTopologyNode_in);

  DWORD number_of_outputs = 0;
  HRESULT result = IMFTopologyNode_in->GetOutputCount (&number_of_outputs);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::GetOutputCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  DWORD input_index = 0;
  for (DWORD i = 0;
       i < number_of_outputs;
       ++i)
  {
    result = IMFTopologyNode_in->GetOutput (i,
                                            &topology_node_p,
                                            &input_index);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::GetOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF

    result = IMFTopologyNode_in->DisconnectOutput (i);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::DisconnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      topology_node_p->Release ();

      return false;
    } // end IF

    if (Stream_Module_Device_Tools::disconnect (topology_node_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), aborting\n")));

      // clean up
      topology_node_p->Release ();

      return false;
    } // end IF
    topology_node_p->Release ();
  } // end FOR

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

//bool
//Stream_Module_Device_Tools::getCaptureFormat (IMFSourceReader* sourceReader_in,
//                                              IMFMediaType*& mediaType_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getCaptureFormat"));
//
//  // sanity check(s)
//  ACE_ASSERT (sourceReader_in);
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  HRESULT result =
//    sourceReader_in->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                          &mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//  ACE_ASSERT (mediaType_out);
//
//  return true;
//}
//
//bool
//Stream_Module_Device_Tools::getOutputFormat (IMFSourceReader* sourceReader_in,
//                                             IMFMediaType*& mediaType_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getOutputFormat"));
//
//  // sanity check(s)
//  ACE_ASSERT (sourceReader_in);
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  HRESULT result = MFCreateMediaType (&mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//
//  result =
//    sourceReader_in->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                          &mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (mediaType_out);
//
//  return true;
//
//error:
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  return false;
//}
bool
Stream_Module_Device_Tools::getCaptureFormat (IMFMediaSource* IMFMediaSource_in,
                                              IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaSource_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  HRESULT result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;
  result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_type_handler_p->Release ();
  media_type_handler_p = NULL;

  return true;

error:
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_Tools::getOutputFormat (IMFTransform* IMFTransform_in,
                                             IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = S_OK;
  DWORD number_of_input_streams = 0;
  DWORD number_of_output_streams = 0;
  result = IMFTransform_in->GetStreamCount (&number_of_input_streams,
                                            &number_of_output_streams);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetStreamCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  DWORD* input_stream_ids_p = NULL;
  ACE_NEW_NORETURN (input_stream_ids_p,
                    DWORD[number_of_input_streams]);
  DWORD* output_stream_ids_p = NULL;
  ACE_NEW_NORETURN (output_stream_ids_p,
                    DWORD[number_of_output_streams]);
  if (!input_stream_ids_p || !output_stream_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    goto error;
  } // end IF
  result = IMFTransform_in->GetStreamIDs (number_of_input_streams,
                                          input_stream_ids_p,
                                          number_of_output_streams,
                                          output_stream_ids_p);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetStreamIDs(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    int i = 0;
    for (;
         i < static_cast<int> (number_of_input_streams);
         ++i)
      input_stream_ids_p[i] = i;
    for (i = 0;
         i < static_cast<int> (number_of_output_streams);
         ++i)
      output_stream_ids_p[i] = i;
  } // end IF
  delete[] input_stream_ids_p;
  input_stream_ids_p = NULL;

  result = IMFTransform_in->GetOutputAvailableType (output_stream_ids_p[0],
                                                    0,
                                                    &IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetOutputAvailableType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  delete[] output_stream_ids_p;

  return true;

error:
  if (input_stream_ids_p)
    delete[] input_stream_ids_p;
  if (output_stream_ids_p)
    delete[] output_stream_ids_p;
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_Tools::getOutputFormat (IMFTopology* IMFTopology_in,
                                             TOPOID nodeId_in,
                                             IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  if (nodeId_in)
  {
    result = IMFTopology_in->GetNodeByID (nodeId_in,
                                          &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                  nodeId_in,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    goto continue_;
  } // end IF

  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_nodes = 0;
  result = collection_p->GetElementCount (&number_of_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  if (number_of_nodes <= 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology contains no output nodes, continuing\n")));
    collection_p->Release ();
    collection_p = NULL;
    result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = collection_p->GetElementCount (&number_of_nodes);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_nodes <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("topology contains no source nodes, aborting\n")));

      // clean up
      collection_p->Release ();

      goto error;
    } // end IF
    result = collection_p->GetElement (0, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    collection_p->Release ();
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();

    IMFTopologyNode* topology_node_2 = NULL;
    DWORD input_index = 0;
    do
    {
      result = topology_node_p->GetOutputCount (&number_of_nodes);
      ACE_ASSERT (SUCCEEDED (result));
      if (number_of_nodes <= 0) break;

      topology_node_2 = NULL;
      result = topology_node_p->GetOutput (0,
                                           &topology_node_2,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      topology_node_p->Release ();
      topology_node_p = topology_node_2;
    } while (true);

    goto continue_;
  } // end IF

  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();

    goto error;
  } // end IF
  unknown_p->Release ();

continue_:
  if (!topology_node_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("output node not found [id was: %q], aborting\n"),
                nodeId_in));
    goto error;
  } // end IF

  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  result = topology_node_p->GetNodeType (&node_type);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_ASSERT (unknown_p);

      IMFStreamSink* stream_sink_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFStreamSink): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        unknown_p->Release ();

        goto error;
      } // end IF
      unknown_p->Release ();
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFStreamSink::GetMediaTypeHandler(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        stream_sink_p->Release ();

        goto error;
      } // end IF
      stream_sink_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        media_type_handler_p->Release ();

        goto error;
      } // end IF
      media_type_handler_p->Release ();
      break;
    }
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_p->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result));
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result));
      stream_descriptor_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release ();
      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_ASSERT (unknown_p);

      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        unknown_p->Release ();

        goto error;
      } // end IF
      unknown_p->Release ();
      result = transform_p->GetOutputCurrentType (0,
                                                  &IMFMediaType_out);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        transform_p->Release ();

        goto error;
      } // end IF
      transform_p->Release ();
      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_p->GetOutputPrefType (0,
                                                   &IMFMediaType_out);
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (IMFMediaType_out);
  topology_node_p->Release ();

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  return false;
}

//bool
//Stream_Module_Device_Tools::setOutputFormat (IMFSourceReader* IMFSourceReader_in,
//                                             const IMFMediaType* IMFMediaType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setOutputFormat"));
//
//  // sanit ycheck(s)
//  ACE_ASSERT (IMFSourceReader_in);
//  ACE_ASSERT (IMFMediaType_in);
//
//  HRESULT result =
//    IMFSourceReader_in->SetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                             NULL,
//                                             const_cast<IMFMediaType*> (IMFMediaType_in));
//  if (FAILED (result)) // MF_E_INVALIDMEDIATYPE: 0xC00D36B4L
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::SetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//
//  return true;
//}
std::string
Stream_Module_Device_Tools::nodeTypeToString (enum MF_TOPOLOGY_TYPE nodeType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::nodeTypeToString"));

  std::string result;

  switch (nodeType_in)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("output"); break;
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("source"); break;
    case MF_TOPOLOGY_TRANSFORM_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("transform"); break;
    case MF_TOPOLOGY_TEE_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("tee"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %u), aborting\n"),
                  nodeType_in));
      break;
    }
  } // end SWITCH

  return result;
}
std::string
Stream_Module_Device_Tools::topologyStatusToString (MF_TOPOSTATUS topologyStatus_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::topologyStatusToString"));

  std::string result;

  switch (topologyStatus_in)
  {
    case MF_TOPOSTATUS_INVALID:
      result = ACE_TEXT_ALWAYS_CHAR ("invalid"); break;
    case MF_TOPOSTATUS_READY:
      result = ACE_TEXT_ALWAYS_CHAR ("ready"); break;
    case MF_TOPOSTATUS_STARTED_SOURCE:
      result = ACE_TEXT_ALWAYS_CHAR ("started"); break;
#if (WINVER >= _WIN32_WINNT_WIN7)
    case MF_TOPOSTATUS_DYNAMIC_CHANGED:
      result = ACE_TEXT_ALWAYS_CHAR ("changed"); break;
#endif
    case MF_TOPOSTATUS_SINK_SWITCHED:
      result = ACE_TEXT_ALWAYS_CHAR ("switched"); break;
    case MF_TOPOSTATUS_ENDED:
      result = ACE_TEXT_ALWAYS_CHAR ("ended"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown topology status (was: %u), aborting\n"),
                  topologyStatus_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
Stream_Module_Device_Tools::activateToString (IMFActivate* IMFActivate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::activateToString"));

  std::string result;

  //IMFAttributes* attributes_p = NULL;
  HRESULT result_2 = E_FAIL;
  //  const_cast<IMFActivate*> (IMFActivate_in)->GetAttributes (&attributes_p);
  //if (FAILED (result_2))
  //{
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("failed to IMFActivate::GetAttributes(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  //  goto error;
  //} // end IF
  WCHAR buffer[BUFSIZ];
  //result_2 = attributes_p->GetString (MFT_FRIENDLY_NAME_Attribute,
  result_2 = IMFActivate_in->GetString (MFT_FRIENDLY_NAME_Attribute,
                                        buffer, sizeof (buffer),
                                        NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to IMFAttributes::GetString(MFT_FRIENDLY_NAME_Attribute): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer));

error:
  //if (attributes_p)
  //  attributes_p->Release ();

  return result;
}

//bool
//Stream_Module_Device_Tools::setCaptureFormat (IMFSourceReaderEx* IMFSourceReaderEx_in,
//                                              const IMFMediaType* IMFMediaType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setCaptureFormat"));
//
//  // sanit ycheck(s)
//  ACE_ASSERT (IMFSourceReaderEx_in);
//  ACE_ASSERT (IMFMediaType_in);
//
//  HRESULT result = E_FAIL;
//  struct _GUID GUID_s = { 0 };
//  UINT32 width, height;
//  UINT32 numerator, denominator;
//  result =
//    const_cast<IMFMediaType*> (IMFMediaType_in)->GetGUID (MF_MT_SUBTYPE,
//                                                          &GUID_s);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//  result = MFGetAttributeSize (const_cast<IMFMediaType*> (IMFMediaType_in),
//                               MF_MT_FRAME_SIZE,
//                               &width, &height);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//  result = MFGetAttributeRatio (const_cast<IMFMediaType*> (IMFMediaType_in),
//                                MF_MT_FRAME_RATE,
//                                &numerator, &denominator);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//
//  DWORD count = 0;
//  IMFMediaType* media_type_p = NULL;
//  struct _GUID GUID_2 = { 0 };
//  UINT32 width_2, height_2;
//  UINT32 numerator_2, denominator_2;
//  DWORD flags = 0;
//  while (result == S_OK)
//  {
//    media_type_p = NULL;
//    result =
//      IMFSourceReaderEx_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                                count,
//                                                &media_type_p);
//    if (result != S_OK) break;
//
//    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    if (GUID_s != GUID_2) goto continue_;
//
//    result = MFGetAttributeSize (media_type_p,
//                                 MF_MT_FRAME_SIZE,
//                                 &width_2, &height_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    if (width != width_2) goto continue_;
//
//    result = MFGetAttributeRatio (media_type_p,
//                                  MF_MT_FRAME_RATE,
//                                  &numerator_2, &denominator_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    if ((numerator   != numerator_2)  ||
//        (denominator != denominator_2)) goto continue_;
//
//    result =
//      IMFSourceReaderEx_in->SetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                                media_type_p,
//                                                &flags);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFSourceReader::SetNativeMediaType(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (media_type_p).c_str ()),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    media_type_p->Release ();
//
//    return true;
//
//continue_:
//    media_type_p->Release ();
//
//    ++count;
//  } // end WHILE
//
//  // *NOTE*: this means that the device does not support the requested media
//  //         type 'natively'
//  //         --> try to auto-load a (MFT/DMO) decoder
//  //             see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd389281(v=vs.85).aspx#setting_output_formats
//  if (!Stream_Module_Device_Tools::setOutputFormat (IMFSourceReaderEx_in,
//                                                    IMFMediaType_in))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Module_Device_Tools::setOutputFormat(), aborting\n")));
//    goto error;
//  } // end IF
//
//  return true;
//
//error:
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("the source reader does not support the requested media type (was: \"%s\"), aborting\n"),
//              ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (IMFMediaType_in).c_str ())));
//
//  // debug info
//  Stream_Module_Device_Tools::dump (IMFSourceReaderEx_in);
//
//  return false;
//}
bool
Stream_Module_Device_Tools::setCaptureFormat (IMFTopology* IMFTopology_in,
                                              const IMFMediaType* IMFMediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setCaptureFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (IMFTopology_in);
  ACE_ASSERT (IMFMediaType_in);

  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();

    return false;
  } // end IF
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();

    return false;
  } // end IF
  unknown_p->Release ();

  IMFMediaSource* media_source_p = NULL;
  result = topology_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&media_source_p));
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release ();
  if (!Stream_Module_Device_Tools::setCaptureFormat (media_source_p,
                                                     IMFMediaType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));

    // clean up
    media_source_p->Release ();

    return false;
  } // end IF
  media_source_p->Release ();

  return true;
}
bool
Stream_Module_Device_Tools::setCaptureFormat (IMFMediaSource* IMFMediaSource_in,
                                              const IMFMediaType* IMFMediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::setCaptureFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (IMFMediaSource_in);
  ACE_ASSERT (IMFMediaType_in);

  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  HRESULT result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;
  result =
    media_type_handler_p->SetCurrentMediaType (const_cast<IMFMediaType*> (IMFMediaType_in));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::SetCurrentMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_type_handler_p->Release ();
  media_type_handler_p = NULL;

  return true;

error:
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();

  return false;
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
  mediaType_out =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
  if (!mediaType_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_OS::memset (mediaType_out, 0, sizeof (struct _AMMediaType));

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

  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  result = isample_grabber_p->GetConnectedMediaType (mediaType_out);
  if (FAILED (result)) // 0x80040209: VFW_E_NOT_CONNECTED
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
                ACE_TEXT ("no capture pin found, aborting\n")));
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
  ACE_ASSERT (stream_config_p);

  pin_p->Release ();

  result =
    stream_config_p->SetFormat (&const_cast<struct _AMMediaType&> (mediaType_in));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::SetFormat(): \"%s\" (0x%x) (media type was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ()), result,
                ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (mediaType_in).c_str ())));

    // clean up
    stream_config_p->Release ();

    return false;
  } // end IF
  stream_config_p->Release ();

  //struct _AMMediaType* media_type_p;
  //stream_config_p->GetFormat (&media_type_p);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("set capture format: %s\n"),
              ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (mediaType_in).c_str ())));

  return true;
}

bool
Stream_Module_Device_Tools::copyAttribute (const IMFAttributes* source_in,
                                           IMFAttributes* destination_in,
                                           const struct _GUID& key_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::copyAttribute"));

  // sanity check(s)
  ACE_ASSERT (source_in);
  ACE_ASSERT (destination_in);

  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);

  HRESULT result =
    const_cast<IMFAttributes*> (source_in)->GetItem (key_in, &property_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::GetItem(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto clean;
  } // end IF
  result = destination_in->SetItem (key_in, property_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::SetItem(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto clean;
  } // end IF

clean:
  PropVariantClear (&property_s);

  return (result == S_OK);
}
bool
Stream_Module_Device_Tools::copyMediaType (const IMFMediaType* IMFMediaType_in,
                                           IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::copyMediaType"));

  // sanity check(s)
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = MFCreateMediaType (&IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  result =
    const_cast<IMFMediaType*> (IMFMediaType_in)->CopyAllItems (IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::CopyAllItems(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;

    return false;
  } // end IF

  return true;
}

bool
Stream_Module_Device_Tools::copyMediaType (const struct _AMMediaType& mediaType_in,
                                           struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::copyMediaType"));

  bool free_memory = false;

  // sanity check(s)
  if (mediaType_out)
    FreeMediaType (*mediaType_out);
  else
  {
    mediaType_out =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    if (!mediaType_out)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, aborting\n")));
      return false;
    } // end IF

    free_memory = true;
  } // end ELSE

  HRESULT result = CopyMediaType (mediaType_out,
                                  &mediaType_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CopyMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    if (free_memory)
    {
      CoTaskMemFree (mediaType_out);
      mediaType_out = NULL;
    } // end IF

    return false;
  } // end IF

  return true;
}
void
Stream_Module_Device_Tools::deleteMediaType (struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::deleteMediaType"));

  //// sanity check(s)
  //if (!mediaType_inout)
  //  return;

  //Stream_Module_Device_Tools::freeMediaType (*mediaType_inout);

  //CoTaskMemFree (mediaType_inout);
  DeleteMediaType (mediaType_inout);

  mediaType_inout = NULL;
}
void
Stream_Module_Device_Tools::freeMediaType (struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::freeMediaType"));

  //if (mediaType_in.cbFormat != 0)
  //{
  //  CoTaskMemFree (static_cast<LPVOID> (mediaType_in.pbFormat));
  //  mediaType_in.cbFormat = 0;
  //  mediaType_in.pbFormat = NULL;
  //} // end IF
  //if (mediaType_in.pUnk)
  //{
  //  // pUnk should not be used.
  //  mediaType_in.pUnk->Release ();
  //  mediaType_in.pUnk = NULL;
  //} // end IF
  FreeMediaType (mediaType_in);
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
    OLECHAR GUID_string[CHARS_IN_GUID];
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    int nCount = StringFromGUID2 (GUID_in,
                                  GUID_string, sizeof (GUID_string));
    ACE_ASSERT (nCount == CHARS_IN_GUID);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    return result;
  } // end IF
  result = (*iterator).second;

  return result;
}

std::string
Stream_Module_Device_Tools::mediaTypeToString (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::mediaTypeToString"));

  std::string result;

  struct _AMMediaType media_type;
  ACE_OS::memset (&media_type, 0, sizeof (media_type));
  HRESULT result_2 =
    MFInitAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (mediaType_in),
                                      GUID_NULL,
                                      &media_type);
  if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFInitAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return std::string ();
  } // end IF

  result = Stream_Module_Device_Tools::mediaTypeToString (media_type);

  // clean up
  Stream_Module_Device_Tools::freeMediaType (media_type);

  return result;
}
std::string
Stream_Module_Device_Tools::mediaTypeToString (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_Tools::mediaTypeToString"));

  std::string result;

  OLECHAR GUID_string[CHARS_IN_GUID];
  ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
  int count = -1;
  GUID2STRING_MAP_ITERATOR_T iterator =
    Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.find (mediaType_in.majortype);
  if (iterator == Stream_Module_Device_Tools::Stream_MediaMajorType2StringMap.end ())
  {
    count = StringFromGUID2 (mediaType_in.majortype,
                             GUID_string, sizeof (GUID_string));
    ACE_ASSERT (count == CHARS_IN_GUID);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media majortype (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    return std::string ();
  } // end IF
  result = ACE_TEXT_ALWAYS_CHAR ("majortype: \"");
  result += (*iterator).second;
  result += ACE_TEXT_ALWAYS_CHAR ("\"\nsubtype: \"");
  result += Stream_Module_Device_Tools::mediaSubTypeToString (mediaType_in.subtype);

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
    ACE_ASSERT (count == CHARS_IN_GUID);
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
    ACE_ASSERT (count == CHARS_IN_GUID);
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
