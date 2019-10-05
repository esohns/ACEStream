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

#include "ace/Synch.h"
#include "stream_lib_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <amvideo.h>
#include <d3d9.h>
#include <initguid.h> // *NOTE*: this exports DEFINE_GUIDs (see e.g. dxva.h)
#include <dvdmedia.h>
#include <dxva.h>
#include <fourcc.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#include <mfapi.h>
#include <wmcodecdsp.h>
#else
#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif /* __cplusplus */

#include "X11/Xlib.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#else
#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

// initialize statics
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap;
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap;
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap;
#endif // ACE_WIN32 || ACE_WIN64

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Stream_MediaFramework_Tools::initialize (enum Stream_MediaFramework_Type mediaFramework_in)
#else
Stream_MediaFramework_Tools::initialize ()
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::initialize"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (unlikely (!Stream_MediaFramework_DirectShow_Tools::initialize (true)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::initialize(), aborting\n")));
        return false;
      } // end IF

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Stream_MediaFramework_MediaFoundation_Tools::initialize ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      return false;
    }
  } // end SWITCH

  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_None, ACE_TEXT_ALWAYS_CHAR ("None")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_VideoInfo, ACE_TEXT_ALWAYS_CHAR ("VideoInfo")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_VideoInfo2, ACE_TEXT_ALWAYS_CHAR ("VideoInfo2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_WaveFormatEx, ACE_TEXT_ALWAYS_CHAR ("WaveFormatEx")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_MPEGVideo, ACE_TEXT_ALWAYS_CHAR ("MPEGVideo")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_MPEGStreams, ACE_TEXT_ALWAYS_CHAR ("MPEGStreams")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_DvInfo, ACE_TEXT_ALWAYS_CHAR ("DvInfo")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_525WSS, ACE_TEXT_ALWAYS_CHAR ("525WSS")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_MPEG2_VIDEO, ACE_TEXT_ALWAYS_CHAR ("MPEG2_VIDEO")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_VIDEOINFO2, ACE_TEXT_ALWAYS_CHAR ("VIDEOINFO2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_MPEG2Video, ACE_TEXT_ALWAYS_CHAR ("MPEG2Video")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_DolbyAC3, ACE_TEXT_ALWAYS_CHAR ("DolbyAC3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_MPEG2Audio, ACE_TEXT_ALWAYS_CHAR ("MPEG2Audio")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_DVD_LPCMAudio, ACE_TEXT_ALWAYS_CHAR ("DVD_LPCMAudio")));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_UVCH264Video, ACE_TEXT_ALWAYS_CHAR ("UVCH264Video")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_JPEGImage, ACE_TEXT_ALWAYS_CHAR ("JPEGImage")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.insert (std::make_pair (FORMAT_Image, ACE_TEXT_ALWAYS_CHAR ("Image")));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  // DirectShow
  //////////////////////////////////////// AUDIO
  // uncompressed audio
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));

  // MPEG-4 and AAC
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_ADTS_AAC")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR("MPEG_HEAAC")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_LOAS, ACE_TEXT_ALWAYS_CHAR("MPEG_LOAS")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_AAC1, ACE_TEXT_ALWAYS_CHAR("RAW_AAC1")));

  // Dolby
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_DDPLUS, ACE_TEXT_ALWAYS_CHAR("DOLBY_DDPLUS")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVM, ACE_TEXT_ALWAYS_CHAR("DVM")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RAW_SPORT, ACE_TEXT_ALWAYS_CHAR("RAW_SPORT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_SPDIF_TAG_241h, ACE_TEXT_ALWAYS_CHAR("SPDIF_TAG_241h")));

  // miscellaneous
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DRM_Audio, ACE_TEXT_ALWAYS_CHAR("DRM_Audio")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DTS2, ACE_TEXT_ALWAYS_CHAR("DTS2")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_PCMAudio_Obsolete, ACE_TEXT_ALWAYS_CHAR("PCMAudio_Obsolete")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR("MPEG_RAW_AAC")));

  /////////////////////////////////////// BDA
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_None, ACE_TEXT_ALWAYS_CHAR("None")));

  /////////////////////////////////////// DVD
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DTS, ACE_TEXT_ALWAYS_CHAR("DTS")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_SUBPICTURE, ACE_TEXT_ALWAYS_CHAR("DVD_SUBPICTURE")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_SDDS, ACE_TEXT_ALWAYS_CHAR("SDDS")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_DSI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_DSI")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PCI, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PCI")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, ACE_TEXT_ALWAYS_CHAR("DVD_NAVIGATION_PROVIDER")));

  /////////////////////////////////////// Line 21
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_BytePair, ACE_TEXT_ALWAYS_CHAR("Line21_BytePair")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_GOPPacket, ACE_TEXT_ALWAYS_CHAR("Line21_GOPPacket")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Line21_VBIRawData, ACE_TEXT_ALWAYS_CHAR("Line21_VBIRawData")));

  /////////////////////////////////////// MPEG-1
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Packet, ACE_TEXT_ALWAYS_CHAR("MPEG1Packet")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Payload, ACE_TEXT_ALWAYS_CHAR("MPEG1Payload")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1AudioPayload, ACE_TEXT_ALWAYS_CHAR("MPEG1AudioPayload")));

  /////////////////////////////////////// MPEG-2
  // MPEG-2 (splitter)
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_VIDEO, ACE_TEXT_ALWAYS_CHAR("MPEG2_VIDEO")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR("DOLBY_AC3_SPDIF")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_AUDIO, ACE_TEXT_ALWAYS_CHAR("MPEG2_AUDIO")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVD_LPCM_AUDIO, ACE_TEXT_ALWAYS_CHAR("DVD_LPCM_AUDIO")));
  // MPEG-2 (demultiplexer)
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_PROGRAM, ACE_TEXT_ALWAYS_CHAR("MPEG2_PROGRAM")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE, ACE_TEXT_ALWAYS_CHAR("MPEG2_TRANSPORT_STRIDE")));
// *NOTE*: see ksuuids.h
#if ( (NTDDI_VERSION >= NTDDI_WINXPSP2) && (NTDDI_VERSION < NTDDI_WS03) ) || (NTDDI_VERSION >= NTDDI_WS03SP1)
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ATSC_SI, ACE_TEXT_ALWAYS_CHAR("ATSC_SI")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVB_SI, ACE_TEXT_ALWAYS_CHAR("DVB_SI")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ISDB_SI, ACE_TEXT_ALWAYS_CHAR("ISDB_SI")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG2DATA, ACE_TEXT_ALWAYS_CHAR("MPEG2DATA")));
#endif
  // MPEG-2 (kernel)

  /////////////////////////////////////// Stream
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AIFF, ACE_TEXT_ALWAYS_CHAR("AIFF")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Asf, ACE_TEXT_ALWAYS_CHAR("Asf")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Avi, ACE_TEXT_ALWAYS_CHAR("Avi")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AU, ACE_TEXT_ALWAYS_CHAR("AU")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DssAudio, ACE_TEXT_ALWAYS_CHAR("DssAudio")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DssVideo, ACE_TEXT_ALWAYS_CHAR("DssVideo")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Audio, ACE_TEXT_ALWAYS_CHAR("MPEG1Audio")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1System, ACE_TEXT_ALWAYS_CHAR("MPEG1System")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1SystemStream, ACE_TEXT_ALWAYS_CHAR("MPEG1SystemStream")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1Video, ACE_TEXT_ALWAYS_CHAR("MPEG1Video")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MPEG1VideoCD, ACE_TEXT_ALWAYS_CHAR("MPEG1VideoCD")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_WAVE, ACE_TEXT_ALWAYS_CHAR("WAVE")));

  /////////////////////////////////////// VBI
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_RAW8, ACE_TEXT_ALWAYS_CHAR("RAW8")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_TELETEXT, ACE_TEXT_ALWAYS_CHAR("TELETEXT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_VPS, ACE_TEXT_ALWAYS_CHAR("VPS")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_WSS, ACE_TEXT_ALWAYS_CHAR("WSS")));

  /////////////////////////////////////// VIDEO
  // analog video
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_NTSC_M, ACE_TEXT_ALWAYS_CHAR("NTSC_M")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_B, ACE_TEXT_ALWAYS_CHAR("PAL_B")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_D, ACE_TEXT_ALWAYS_CHAR("PAL_D")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_G, ACE_TEXT_ALWAYS_CHAR("PAL_G")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_H, ACE_TEXT_ALWAYS_CHAR("PAL_H")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_I, ACE_TEXT_ALWAYS_CHAR("PAL_I")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_M, ACE_TEXT_ALWAYS_CHAR("PAL_M")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_PAL_N, ACE_TEXT_ALWAYS_CHAR("PAL_N")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_B, ACE_TEXT_ALWAYS_CHAR("SECAM_B")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_D, ACE_TEXT_ALWAYS_CHAR("SECAM_D")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_G, ACE_TEXT_ALWAYS_CHAR("SECAM_G")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_H, ACE_TEXT_ALWAYS_CHAR("SECAM_H")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K, ACE_TEXT_ALWAYS_CHAR("SECAM_K")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_K1, ACE_TEXT_ALWAYS_CHAR("SECAM_K1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AnalogVideo_SECAM_L, ACE_TEXT_ALWAYS_CHAR("SECAM_L")));

  // directx video acceleration
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AI44, ACE_TEXT_ALWAYS_CHAR("AI44")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IA44, ACE_TEXT_ALWAYS_CHAR("IA44")));

  // DV video
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_dvsl, ACE_TEXT_ALWAYS_CHAR("dvsl")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_dvsd, ACE_TEXT_ALWAYS_CHAR("dvsd")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_dvhd, ACE_TEXT_ALWAYS_CHAR("dvhd")));

  // H.264
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AVC1, ACE_TEXT_ALWAYS_CHAR("AVC1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_H264, ACE_TEXT_ALWAYS_CHAR("H264")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_h264, ACE_TEXT_ALWAYS_CHAR("h264")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_X264, ACE_TEXT_ALWAYS_CHAR("X264")));
  //Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_x264, ACE_TEXT_ALWAYS_CHAR("x264")));

  // uncompressed RGB (no alpha)
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB1, ACE_TEXT_ALWAYS_CHAR("RGB1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB4, ACE_TEXT_ALWAYS_CHAR("RGB4")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB8, ACE_TEXT_ALWAYS_CHAR("RGB8")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB555, ACE_TEXT_ALWAYS_CHAR("RGB555")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB565, ACE_TEXT_ALWAYS_CHAR("RGB565")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB24, ACE_TEXT_ALWAYS_CHAR("RGB24")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32, ACE_TEXT_ALWAYS_CHAR("RGB32")));
  // uncompressed RGB (alpha)
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555, ACE_TEXT_ALWAYS_CHAR("ARGB1555")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32, ACE_TEXT_ALWAYS_CHAR("ARGB32")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444, ACE_TEXT_ALWAYS_CHAR("ARGB4444")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_A2R10G10B10, ACE_TEXT_ALWAYS_CHAR("A2R10G10B10")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_A2B10G10R10, ACE_TEXT_ALWAYS_CHAR("A2B10G10R10")));

  // video mixing renderer (VMR-7)
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX7_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX7_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX7_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX7_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX7_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX7_RT")));
  // video mixing renderer (VMR-9)
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB32_D3D_DX9_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_RGB16_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("RGB16_D3D_DX9_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB32_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB32_D3D_DX9_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB4444_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB4444_D3D_DX9_RT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_ARGB1555_D3D_DX9_RT, ACE_TEXT_ALWAYS_CHAR("ARGB1555_D3D_DX9_RT")));

  // YUV video
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_AYUV, ACE_TEXT_ALWAYS_CHAR("AYUV")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_YUY2, ACE_TEXT_ALWAYS_CHAR("YUY2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_UYVY, ACE_TEXT_ALWAYS_CHAR("UYVY")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IMC1, ACE_TEXT_ALWAYS_CHAR("IMC1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IMC2, ACE_TEXT_ALWAYS_CHAR("IMC2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IMC3, ACE_TEXT_ALWAYS_CHAR("IMC3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IMC4, ACE_TEXT_ALWAYS_CHAR("IMC4")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_YV12, ACE_TEXT_ALWAYS_CHAR("YV12")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_NV12, ACE_TEXT_ALWAYS_CHAR("NV12")));
  // other YUV
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_I420, ACE_TEXT_ALWAYS_CHAR("I420")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IF09, ACE_TEXT_ALWAYS_CHAR("IF09")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IYUV, ACE_TEXT_ALWAYS_CHAR("IYUV")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Y211, ACE_TEXT_ALWAYS_CHAR("Y211")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Y411, ACE_TEXT_ALWAYS_CHAR("Y411")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Y41P, ACE_TEXT_ALWAYS_CHAR("Y41P")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_YVU9, ACE_TEXT_ALWAYS_CHAR("YVU9")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_YVYU, ACE_TEXT_ALWAYS_CHAR("YVYU")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_YUYV, ACE_TEXT_ALWAYS_CHAR("YUYV")));

  // miscellaneous
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_CFCC, ACE_TEXT_ALWAYS_CHAR("CFCC")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_CLJR, ACE_TEXT_ALWAYS_CHAR("CLJR")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_CPLA, ACE_TEXT_ALWAYS_CHAR("CPLA")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_CLPL, ACE_TEXT_ALWAYS_CHAR("CLPL")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_IJPG, ACE_TEXT_ALWAYS_CHAR("IJPG")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MDVF, ACE_TEXT_ALWAYS_CHAR("MDVF")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_MJPG, ACE_TEXT_ALWAYS_CHAR("MJPG")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Overlay, ACE_TEXT_ALWAYS_CHAR("Overlay")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_Plum, ACE_TEXT_ALWAYS_CHAR("Plum")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_QTJpeg, ACE_TEXT_ALWAYS_CHAR("QTJpeg")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_QTMovie, ACE_TEXT_ALWAYS_CHAR("QTMovie")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_QTRle, ACE_TEXT_ALWAYS_CHAR("QTRle")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_QTRpza, ACE_TEXT_ALWAYS_CHAR("QTRpza")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_QTSmc, ACE_TEXT_ALWAYS_CHAR("QTSmc")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_TVMJ, ACE_TEXT_ALWAYS_CHAR("TVMJ")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_VPVBI, ACE_TEXT_ALWAYS_CHAR("VPVBI")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_VPVideo, ACE_TEXT_ALWAYS_CHAR("VPVideo")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_WAKE, ACE_TEXT_ALWAYS_CHAR("WAKE")));

  ///////////////////////////////////////
  // unknown
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVCS, ACE_TEXT_ALWAYS_CHAR("DVCS")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (MEDIASUBTYPE_DVSD, ACE_TEXT_ALWAYS_CHAR("DVSD")));

  ///////////////////////////////////////
  // DirectX VA
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeNone, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeNone")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH261_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH261_A")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH261_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH261_B")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH263_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_A")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH263_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_B")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH263_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_C")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH263_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_D")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH263_E, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_E")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH263_F, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH263_F")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG1_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG1_A")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG1_VLD, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG1_VLD")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG2_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_A")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG2_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_B")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG2_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_C")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG2_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2_D")));

#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG2and1_VLD, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG2and1_VLD")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_MoComp_NoFGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_MoComp_FGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_IDCT_NoFGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_IDCT_FGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_E, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_NoFGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_F, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_FGT")));

#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_VLD_WithFMOASO_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_WithFMOASO_NoFGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_VLD_Stereo_Progressive_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_Stereo_Progressive_NoFGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_VLD_Stereo_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_Stereo_NoFGT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeH264_VLD_Multiview_NoFGT, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeH264_VLD_Multiview_NoFGT")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeWMV8_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV8_PostProc")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeWMV8_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV8_MoComp")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeWMV9_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV9_PostProc")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeWMV9_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV9_MoComp")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeWMV9_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeWMV9_IDCT")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeVC1_A, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_PostProc")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeVC1_B, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_MoComp")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeVC1_C, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_IDCT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeVC1_D, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_VLD")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeVC1_D2010, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeVC1_D2010")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8

#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG4pt2_VLD_Simple, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG4pt2_VLD_Simple")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeHEVC_VLD_Main, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeHEVC_VLD_Main")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_ModeHEVC_VLD_Main10, ACE_TEXT_ALWAYS_CHAR ("DXVA_ModeHEVC_VLD_Main10")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8

  Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.insert (std::make_pair (DXVA_NoEncrypt, ACE_TEXT_ALWAYS_CHAR ("DXVA_NoEncrypt")));

  //////////////////////////////////////////////////////////////////////////////

  // Media Foundation
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_RGB32, ACE_TEXT_ALWAYS_CHAR ("RGB32")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_ARGB32, ACE_TEXT_ALWAYS_CHAR ("ARGB32")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_RGB24, ACE_TEXT_ALWAYS_CHAR ("RGB24")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_RGB555, ACE_TEXT_ALWAYS_CHAR ("RGB555")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_RGB565, ACE_TEXT_ALWAYS_CHAR ("RGB565")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_RGB8, ACE_TEXT_ALWAYS_CHAR ("RGB8")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_AI44, ACE_TEXT_ALWAYS_CHAR ("AI44")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_AYUV, ACE_TEXT_ALWAYS_CHAR ("AYUV")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_YUY2, ACE_TEXT_ALWAYS_CHAR ("YUY2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_YVYU, ACE_TEXT_ALWAYS_CHAR ("YVYU")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_YVU9, ACE_TEXT_ALWAYS_CHAR ("YVU9")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_UYVY, ACE_TEXT_ALWAYS_CHAR ("UYVY")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_NV11, ACE_TEXT_ALWAYS_CHAR ("NV11")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_NV12, ACE_TEXT_ALWAYS_CHAR ("NV12")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_YV12, ACE_TEXT_ALWAYS_CHAR ("YV12")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_I420, ACE_TEXT_ALWAYS_CHAR ("I420")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_IYUV, ACE_TEXT_ALWAYS_CHAR ("IYUV")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Y210, ACE_TEXT_ALWAYS_CHAR ("Y210")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Y216, ACE_TEXT_ALWAYS_CHAR ("Y216")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Y410, ACE_TEXT_ALWAYS_CHAR ("Y410")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Y416, ACE_TEXT_ALWAYS_CHAR ("Y416")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Y41P, ACE_TEXT_ALWAYS_CHAR ("Y41P")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Y41T, ACE_TEXT_ALWAYS_CHAR ("Y41T")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Y42T, ACE_TEXT_ALWAYS_CHAR ("Y42T")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_P210, ACE_TEXT_ALWAYS_CHAR ("P210")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_P216, ACE_TEXT_ALWAYS_CHAR ("P216")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_P010, ACE_TEXT_ALWAYS_CHAR ("P010")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_P016, ACE_TEXT_ALWAYS_CHAR ("P016")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_v210, ACE_TEXT_ALWAYS_CHAR ("V210")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_v216, ACE_TEXT_ALWAYS_CHAR ("V216")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_v410, ACE_TEXT_ALWAYS_CHAR ("V410")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MP43, ACE_TEXT_ALWAYS_CHAR ("MP43")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MP4S, ACE_TEXT_ALWAYS_CHAR ("MP4S")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_M4S2, ACE_TEXT_ALWAYS_CHAR ("M4S2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MP4V, ACE_TEXT_ALWAYS_CHAR ("MP4V")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_WMV1, ACE_TEXT_ALWAYS_CHAR ("WMV1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_WMV2, ACE_TEXT_ALWAYS_CHAR ("WMV2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_WMV3, ACE_TEXT_ALWAYS_CHAR ("WMV3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_WVC1, ACE_TEXT_ALWAYS_CHAR ("WVC1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MSS1, ACE_TEXT_ALWAYS_CHAR ("MSS1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MSS2, ACE_TEXT_ALWAYS_CHAR ("MSS2")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MPG1, ACE_TEXT_ALWAYS_CHAR ("MPG1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_DVSL, ACE_TEXT_ALWAYS_CHAR ("DVSL")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_DVSD, ACE_TEXT_ALWAYS_CHAR ("DVSD")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_DVHD, ACE_TEXT_ALWAYS_CHAR ("DVHD")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_DV25, ACE_TEXT_ALWAYS_CHAR ("DV25")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_DV50, ACE_TEXT_ALWAYS_CHAR ("DV50")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_DVH1, ACE_TEXT_ALWAYS_CHAR ("DVH1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_DVC,  ACE_TEXT_ALWAYS_CHAR ("DVC")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_H264, ACE_TEXT_ALWAYS_CHAR ("H264")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MJPG, ACE_TEXT_ALWAYS_CHAR ("MJPG")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_420O, ACE_TEXT_ALWAYS_CHAR ("420O")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_HEVC, ACE_TEXT_ALWAYS_CHAR ("HEVC")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_HEVC_ES, ACE_TEXT_ALWAYS_CHAR ("HEVC_ES")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_H263, ACE_TEXT_ALWAYS_CHAR ("H263")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_H264_ES, ACE_TEXT_ALWAYS_CHAR ("H264_ES")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_MPEG2, ACE_TEXT_ALWAYS_CHAR ("MPEG2")));

  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Float, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS, ACE_TEXT_ALWAYS_CHAR ("DTS")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3_SPDIF")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DRM, ACE_TEXT_ALWAYS_CHAR ("DRM")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_WMAudioV8, ACE_TEXT_ALWAYS_CHAR ("WMAudioV8")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_WMAudioV9, ACE_TEXT_ALWAYS_CHAR ("WMAudioV9")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_WMAudio_Lossless, ACE_TEXT_ALWAYS_CHAR ("WMAudio_Lossless")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_WMASPDIF, ACE_TEXT_ALWAYS_CHAR ("WMASPDIF")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_MSP1, ACE_TEXT_ALWAYS_CHAR ("MSP1")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_MP3, ACE_TEXT_ALWAYS_CHAR ("MP3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_MPEG, ACE_TEXT_ALWAYS_CHAR ("MPEG")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_AAC, ACE_TEXT_ALWAYS_CHAR ("AAC")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_ADTS, ACE_TEXT_ALWAYS_CHAR ("ADTS")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_AMR_NB, ACE_TEXT_ALWAYS_CHAR ("AMR_NB")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("AMR_WB")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_AMR_WP, ACE_TEXT_ALWAYS_CHAR ("AMR_WP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Dolby_DDPlus, ACE_TEXT_ALWAYS_CHAR ("Dolby_DDPlus")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}
void
Stream_MediaFramework_Tools::finalize (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                       enum Stream_MediaFramework_Type mediaFramework_in)
#else
                                      )
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::finalize"));

}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_MediaFramework_Tools::isRGB (REFGUID subType_in,
                                    enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::isRGB"));

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      return (// uncompressed RGB (no alpha)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB1)                ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB4)                ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB8)                ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB555)              ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB565)              ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB24)               ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32)               ||
              // uncompressed RGB (alpha)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555)            ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32)              ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444)            ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_A2R10G10B10)         ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_A2B10G10R10)         ||
              // video mixing renderer (VMR-7)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX7_RT)    ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB16_D3D_DX7_RT)    ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX7_RT)   ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444_D3D_DX7_RT) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555_D3D_DX7_RT) ||
              // video mixing renderer (VMR-9)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX9_RT)    ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB16_D3D_DX9_RT)    ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX9_RT)   ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444_D3D_DX9_RT) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555_D3D_DX9_RT));
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      return (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB32)  ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_ARGB32) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_RGB24)  ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_RGB555) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_RGB565) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_RGB8));
    default:
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
  } // end SWITCH

  return false;
}
bool
Stream_MediaFramework_Tools::isChromaLuminance (REFGUID subType_in,
                                                enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::isChromaLuminance"));

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      return (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_AYUV) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_YUY2) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_UYVY) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IMC1) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IMC2) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IMC3) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IMC4) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_YV12) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_NV12) ||
              //
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_I420) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IF09) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IYUV) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_Y211) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_Y411) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_Y41P) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_YVU9) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_YVYU) ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_YUYV));
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      //return MFIsFormatYUV ();
      return (InlineIsEqualGUID (subType_in, MFVideoFormat_AYUV) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_YUY2) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_YVYU) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_YVU9) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_UYVY) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_NV11) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_NV12) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_YV12) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_I420) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_IYUV) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_Y210) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_Y216) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_Y410) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_Y416) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_Y41P) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_Y41T) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_Y42T) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_P210) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_P216) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_P010) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_P016) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_v210) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_v216) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_v410));
    default:
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
  } // end SWITCH

  return false;
}

WORD
Stream_MediaFramework_Tools::toBitCount (REFGUID subType_in,
                                         enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::toBitCount"));

  // initialize return value(s)
  WORD result = 0;

  // sanity check(s)
  if (!Stream_MediaFramework_Tools::isRGB (subType_in,
                                           mediaFramework_in))
  { // *TODO*
    //ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (0);
    ACE_NOTREACHED (return 0;)
  } // end IF

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      // uncompressed RGB (no alpha)
      if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB1))
        return 1;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB4))
        return 4;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB8))
        return 8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB555))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB565))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB24))
        return 24;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32))
        return 32;
      // uncompressed RGB (alpha)
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_A2R10G10B10))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_A2B10G10R10))
        return 32;
      // video mixing renderer (VMR-7)
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX7_RT))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB16_D3D_DX7_RT))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX7_RT))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444_D3D_DX7_RT))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555_D3D_DX7_RT))
        return 16;
      // video mixing renderer (VMR-9)
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX9_RT))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB16_D3D_DX9_RT))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX9_RT))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB4444_D3D_DX9_RT))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB1555_D3D_DX9_RT))
        return 16;
      else
      {
        // *TODO*
        ACE_ASSERT (false);
        ACE_NOTSUP_RETURN (0);
        ACE_NOTREACHED (return 0;)
      } // end ELSE
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB32))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_ARGB32))
        return 32;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB24))
        return 24;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB555))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB565))
        return 16;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB8))
        return 8;
      else
      {
        // *TODO*
        ACE_ASSERT (false);
        ACE_NOTSUP_RETURN (0);
        ACE_NOTREACHED (return 0;)
      } // end ELSE
      break;
    default:
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
  } // end SWITCH

  return result;
}

std::string
Stream_MediaFramework_Tools::mediaFormatTypeToString (REFGUID mediaFormatType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::mediaFormatTypeToString"));

  std::string result;

  Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator =
    Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.find (mediaFormatType_in);
  if (iterator == Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaFormatType_in).c_str ())));
    return result;
  } // end IF
  result = (*iterator).second;

  return result;
}

std::string
Stream_MediaFramework_Tools::mediaSubTypeToString (REFGUID mediaSubType_in,
                                                   enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::mediaSubTypeToString"));

  std::string result;

  // within FOURCC range ? --> use helper class
  if ((mediaSubType_in.Data2 == 0x0000) &&
      (mediaSubType_in.Data3 == 0x0010) &&
      ((mediaSubType_in.Data4[0] == 0x80) &&
       (mediaSubType_in.Data4[1] == 0x00) &&
       (mediaSubType_in.Data4[2] == 0x00) &&
       (mediaSubType_in.Data4[3] == 0xAA) &&
       (mediaSubType_in.Data4[4] == 0x00) &&
       (mediaSubType_in.Data4[5] == 0x38) &&
       (mediaSubType_in.Data4[6] == 0x9B) &&
       (mediaSubType_in.Data4[7] == 0x71)))
  {
    FOURCCMap fourcc_map (&mediaSubType_in);

    return Stream_MediaFramework_Tools::FOURCCToString (fourcc_map.GetFOURCC ());
  } // end IF

  Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      iterator =
        Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.find (mediaSubType_in);
      if (unlikely (iterator == Stream_MediaFramework_Tools::Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap.end ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (mediaSubType_in).c_str ())));
        return result;
      } // end IF
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      iterator =
        Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.find (mediaSubType_in);
      if (unlikely (iterator == Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.end ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (mediaSubType_in).c_str ())));
        return result;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      return result;
    }
  } // end SWITCH
  result = (*iterator).second;

  return result;
}

std::string
Stream_MediaFramework_Tools::mediaMajorTypeToString (REFGUID mediaMajorType_in,
                                                     enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::mediaMajorTypeToString"));

  std::string result;

  Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      iterator =
        Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.find (mediaMajorType_in);
      if (unlikely (iterator == Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.end ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media type (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (mediaMajorType_in).c_str ())));
        return result;
      } // end IF
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      iterator =
        Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.find (mediaMajorType_in);
      if (unlikely (iterator == Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.end ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media type (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (mediaMajorType_in).c_str ())));
        return result;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      return result;
    }
  } // end SWITCH
  result = (*iterator).second;

  return result;
}

unsigned int
Stream_MediaFramework_Tools::frameSize (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::frameSize"));

  unsigned int result = 0;

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header_2_p = NULL;
  struct tWAVEFORMATEX* wave_format_ex_p = NULL;

  if (InlineIsEqualGUID (mediaType_in.majortype, MEDIATYPE_Audio))
    goto audio;
  if (InlineIsEqualGUID (mediaType_in.majortype, MEDIATYPE_Video))
    goto video;

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("invalid/unknown media type (was: \"%s\"), aborting\n"),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaMajorTypeToString (mediaType_in.majortype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
  goto continue_;

audio:
  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  { ACE_ASSERT (mediaType_in.pbFormat);
    struct tWAVEFORMATEX* wave_format_ex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (mediaType_in.pbFormat);
    result =
      (wave_format_ex_p->wBitsPerSample / 8) * wave_format_ex_p->nChannels;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return result;
  } // end ELSE

  goto continue_;

video:
  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (mediaType_in.pbFormat);
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
    result = DIBSIZE (video_info_header_p->bmiHeader);
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  { ACE_ASSERT (mediaType_in.pbFormat);
    video_info_header_2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_in.pbFormat);
    result = DIBSIZE (video_info_header_2_p->bmiHeader);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return result;
  } // end ELSE

continue_:
  return result;
}
unsigned int
Stream_MediaFramework_Tools::frameSize (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::frameSize"));

  unsigned int result = 0;

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _GUID GUID_s = GUID_NULL;
  HRESULT result_2 =
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_MAJOR_TYPE,
                                                       &GUID_s);
  ACE_ASSERT (SUCCEEDED (result_2));
  if (InlineIsEqualGUID (GUID_s, MFMediaType_Audio))
    goto audio;
  if (InlineIsEqualGUID (GUID_s, MFMediaType_Video))
    goto video;

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("invalid/unknown media type (was: \"%s\"), aborting\n"),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaMajorTypeToString (GUID_s, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
  goto continue_;

audio:
  ACE_ASSERT (false); // *TODO*
  goto continue_;

video:
  GUID_s = GUID_NULL;
  result_2 =
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_SUBTYPE,
                                                       &GUID_s);
  ACE_ASSERT (SUCCEEDED (result_2));
  ACE_ASSERT (!InlineIsEqualGUID (GUID_s, GUID_NULL));
  UINT32 width, height;
  result_2 = MFGetAttributeSize (const_cast<IMFMediaType*> (mediaType_in),
                                 MF_MT_FRAME_SIZE,
                                 &width, &height);
  ACE_ASSERT (SUCCEEDED (result_2));
  ACE_ASSERT (width && height);
  UINT32 image_size_i = 0;
  result_2 = MFCalculateImageSize (GUID_s,
                                   width, height,
                                   &image_size_i);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCalculateImageSize(\"%s\",%u,%u): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (GUID_s, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
                width, height,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  result = image_size_i;

continue_:
  return result;
}
#else
std::string
Stream_MediaFramework_Tools::toString (const Display& display_in,
                                       int errorCode_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::toString"));

  // initialize return value(s)
  std::string return_value;

  char buffer_a[BUFSIZ];
  ACE_OS::memset (&buffer_a, 0, sizeof (char[BUFSIZ]));

  Status result = XGetErrorText (&const_cast<Display&> (display_in),
                                 errorCode_in,
                                 buffer_a, sizeof (char[BUFSIZ]));
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to XGetErrorText(0x%@,%d): \"%m\", aborting\n"),
                &display_in,
                errorCode_in));
    return return_value;
  } // end IF
  return_value = buffer_a;

  return return_value;
}

Common_Image_Resolution_t
Stream_MediaFramework_Tools::toResolution (const Display& display_in,
                                           Window window_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::toResolution"));

  Common_Image_Resolution_t return_value;

  XWindowAttributes attributes_s;
  ACE_OS::memset (&attributes_s, 0, sizeof (XWindowAttributes));
  Status result = XGetWindowAttributes (&const_cast<Display&> (display_in),
                                        window_in,
                                        &attributes_s);
  if (unlikely (!result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to XGetWindowAttributes(0x%@,%u): \"%s\", aborting\n"),
                &display_in, window_in,
                ACE_TEXT (Stream_MediaFramework_Tools::toString (display_in, result).c_str ())));
    return return_value;
  } // end IF
  return_value.width = attributes_s.width;
  return_value.height = attributes_s.height;

  return return_value;
}

void
Stream_MediaFramework_Tools::ALSAToSoX (enum _snd_pcm_format format_in,
                                        sox_rate_t rate_in,
                                        unsigned channels_in,
                                        struct sox_encodinginfo_t& encoding_out,
                                        struct sox_signalinfo_t& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::ALSAToSoX"));

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
  switch (format_in)
  {
    // PCM 'formats'
    case SND_PCM_FORMAT_S8:
      break;
    case SND_PCM_FORMAT_U8:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S16_BE:
      break;
    case SND_PCM_FORMAT_U16_LE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U16_BE:
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
    case SND_PCM_FORMAT_MU_LAW:
      encoding_out.encoding = SOX_ENCODING_ULAW;
      break;
    case SND_PCM_FORMAT_A_LAW:
      encoding_out.encoding = SOX_ENCODING_ALAW;
      break;
    case SND_PCM_FORMAT_IMA_ADPCM:
      encoding_out.encoding = SOX_ENCODING_IMA_ADPCM;
      break;
    case SND_PCM_FORMAT_GSM:
      encoding_out.encoding = SOX_ENCODING_GSM;
      break;
    case SND_PCM_FORMAT_G723_24:
    case SND_PCM_FORMAT_G723_24_1B:
    case SND_PCM_FORMAT_G723_40:
    case SND_PCM_FORMAT_G723_40_1B:
      encoding_out.encoding = SOX_ENCODING_G723;
      break;
    case SND_PCM_FORMAT_IEC958_SUBFRAME_LE:
    case SND_PCM_FORMAT_IEC958_SUBFRAME_BE:
    case SND_PCM_FORMAT_MPEG:
    case SND_PCM_FORMAT_S20_LE:
    case SND_PCM_FORMAT_S20_BE:
    case SND_PCM_FORMAT_U20_LE:
    case SND_PCM_FORMAT_U20_BE:
    case SND_PCM_FORMAT_SPECIAL:
    case SND_PCM_FORMAT_S24_3LE:
    case SND_PCM_FORMAT_S24_3BE:
    case SND_PCM_FORMAT_U24_3LE:
    case SND_PCM_FORMAT_U24_3BE:
    case SND_PCM_FORMAT_S20_3LE:
    case SND_PCM_FORMAT_S20_3BE:
    case SND_PCM_FORMAT_U20_3LE:
    case SND_PCM_FORMAT_U20_3BE:
    case SND_PCM_FORMAT_S18_3LE:
    case SND_PCM_FORMAT_S18_3BE:
    case SND_PCM_FORMAT_U18_3LE:
    case SND_PCM_FORMAT_U18_3BE:
    case SND_PCM_FORMAT_DSD_U8:
    case SND_PCM_FORMAT_DSD_U16_LE:
    case SND_PCM_FORMAT_DSD_U32_LE:
    case SND_PCM_FORMAT_DSD_U16_BE:
    case SND_PCM_FORMAT_DSD_U32_BE:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ALSA audio frame format (was: %d), continuing\n"),
//                  ALSA_format));
                  format_in));
//      ACE_ASSERT (false);
      encoding_out.encoding = SOX_ENCODING_UNKNOWN;
      break;
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
  encoding_out.bits_per_sample = snd_pcm_format_width (format_in);
  encoding_out.reverse_bytes = sox_option_default;
  encoding_out.reverse_nibbles = sox_option_default;
  encoding_out.reverse_bits = sox_option_default;
  encoding_out.opposite_endian = sox_false;

//  format_out.rate = sample_rate;
//  format_out.channels = channels;
//  format_out.precision = snd_pcm_format_width (ALSA_format);
    format_out.rate = rate_in;
    format_out.channels = channels_in;
    format_out.precision = snd_pcm_format_width (format_in);
//      format_out.length = 0;
//      format_out.mult = NULL;
}

unsigned int
Stream_MediaFramework_Tools::toFrameSize (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::toFrameSize"));

  int result =
      av_image_get_buffer_size (Stream_Device_Tools::v4l2FormatToffmpegFormat (mediaType_in.format.pixelformat),
                                mediaType_in.format.width, mediaType_in.format.height,
                                1); // *TODO*: linesize alignment
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to av_image_get_buffer_size(%u,%u,%u), aborting\n"),
                mediaType_in.format.pixelformat,
                mediaType_in.format.width, mediaType_in.format.height));
    return 0;
  } // end IF

  return static_cast<unsigned int> (result);
}
#endif // ACE_WIN32 || ACE_WIN64
