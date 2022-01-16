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

#include "stream_lib_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define INITGUID // *NOTE*: this exports DEFINE_GUIDs (see e.g. dxva.h)
#include "guiddef.h"
#include "amvideo.h"
#include "d3d9.h"
#include "dvdmedia.h"
#include "dxva.h"
#include "fourcc.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#include "mfapi.h"
#include "wmcodecdsp.h"
#else
#if defined (LIBCAMERA_SUPPORT)
#include "libcamera/libcamera.h"
#endif // LIBCAMERA_SUPPORT
#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif /* __cplusplus */
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_error_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#else
#include "X11/Xlib.h"
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
  Stream_MediaFramework_DirectSound_Tools::initialize ();

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (unlikely (!Stream_MediaFramework_DirectShow_Tools::initialize ()))
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
#if (WINVER >= _WIN32_WINNT_WINTHRESHOLD)
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_FLAC, ACE_TEXT_ALWAYS_CHAR ("FLAC")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_ALAC, ACE_TEXT_ALWAYS_CHAR ("ALAC")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Opus, ACE_TEXT_ALWAYS_CHAR ("Opus")));
#endif // WINVER >= _WIN32_WINNT_WINTHRESHOLD
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Dolby_DDPlus, ACE_TEXT_ALWAYS_CHAR ("Dolby_DDPlus")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Vorbis, ACE_TEXT_ALWAYS_CHAR ("Vorbis")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS_RAW, ACE_TEXT_ALWAYS_CHAR ("DTS_RAW")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS_HD, ACE_TEXT_ALWAYS_CHAR ("DTS_HD")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS_XLL, ACE_TEXT_ALWAYS_CHAR ("DTS_XLL")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS_LBR, ACE_TEXT_ALWAYS_CHAR ("DTS_LBR")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS_UHD, ACE_TEXT_ALWAYS_CHAR ("DTS_UHD")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS_UHDY, ACE_TEXT_ALWAYS_CHAR ("DTS_UHDY")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_DTS_UHDY, ACE_TEXT_ALWAYS_CHAR ("DTS_UHDY")));
#if (NTDDI_VERSION >= NTDDI_WIN10_RS2)
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Float_SpatialObjects, ACE_TEXT_ALWAYS_CHAR ("Float_SpatialObjects")));
#endif // NTDDI_VERSION >= NTDDI_WIN10_RS2
#if (WINVER >= _WIN32_WINNT_THRESHOLD)
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_LPCM, ACE_TEXT_ALWAYS_CHAR ("LPCM")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_PCM_HDCP, ACE_TEXT_ALWAYS_CHAR ("PCM_HDCP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3_HDCP, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3_HDCP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_AAC_HDCP, ACE_TEXT_ALWAYS_CHAR ("AAC_HDCP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_ADTS_HDCP, ACE_TEXT_ALWAYS_CHAR ("ADTS_HDCP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFAudioFormat_Base_HDCP, ACE_TEXT_ALWAYS_CHAR ("Base_HDCP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_H264_HDCP, ACE_TEXT_ALWAYS_CHAR ("H264_HDCP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_HEVC_HDCP, ACE_TEXT_ALWAYS_CHAR ("HEVC_HDCP")));
  Stream_MediaFramework_Tools::Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap.insert (std::make_pair (MFVideoFormat_Base_HDCP, ACE_TEXT_ALWAYS_CHAR ("Base_HDCP")));
#endif // WINVER >= _WIN32_WINNT_THRESHOLD
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
Stream_MediaFramework_Tools::isCompressed (REFGUID subType_in,
                                           REFGUID deviceCategory_in,
                                           enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::isCompressed"));

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
        return Stream_MediaFramework_Tools::isCompressedAudio (subType_in,
                                                               STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
      if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
        return Stream_MediaFramework_Tools::isCompressedVideo (subType_in,
                                                               STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // *TODO*
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
    }
  } // end SWITCH

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
}

bool
Stream_MediaFramework_Tools::isCompressedAudio (REFGUID subType_in,
                                                enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::isCompressedAudio"));

  // *TODO*: this is probably incomplete
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      return (!InlineIsEqualGUID (subType_in, MEDIASUBTYPE_PCM) &&
              !InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IEEE_FLOAT));
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      return (!InlineIsEqualGUID (subType_in, MFAudioFormat_PCM) &&
              !InlineIsEqualGUID (subType_in, MFAudioFormat_Float));
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
    }
  } // end SWITCH

  return false;
}

bool
Stream_MediaFramework_Tools::isCompressedVideo (REFGUID subType_in,
                                                enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::isCompressedVideo"));

  // *TODO*: this is probably incomplete
  return (!Stream_MediaFramework_Tools::isRGB (subType_in,
                                               mediaFramework_in) &&
          !Stream_MediaFramework_Tools::isChromaLuminance (subType_in,
                                                           mediaFramework_in));
}

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
Stream_MediaFramework_Tools::isRGB32 (REFGUID subType_in,
                                      enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::isRGB32"));

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      return (// uncompressed RGB (no alpha)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32)               ||
              // uncompressed RGB (alpha)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32)              ||
              // video mixing renderer (VMR-7)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX7_RT)    ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX7_RT)   ||
              // video mixing renderer (VMR-9)
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32_D3D_DX9_RT)    ||
              InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32_D3D_DX9_RT));
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      return (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB32) ||
              InlineIsEqualGUID (subType_in, MFVideoFormat_ARGB32));
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
    // handle exceptions
    if ((mediaSubType_in == MEDIASUBTYPE_PCM)               ||
        (mediaSubType_in == MEDIASUBTYPE_IEEE_FLOAT)        ||
        (mediaSubType_in == MEDIASUBTYPE_DRM_Audio)         ||
        (mediaSubType_in == MEDIASUBTYPE_MPEG1AudioPayload) ||
        (mediaSubType_in == MEDIASUBTYPE_DOLBY_AC3_SPDIF)   ||
        (mediaSubType_in == MEDIASUBTYPE_RAW_SPORT)         ||
        (mediaSubType_in == MEDIASUBTYPE_SPDIF_TAG_241h))
      goto continue_;

    FOURCCMap fourcc_map (&mediaSubType_in);

    return Stream_MediaFramework_Tools::FOURCCToString (fourcc_map.GetFOURCC ());
  } // end IF

continue_:
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
        return Common_Tools::GUIDToString (mediaSubType_in);
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
        return Common_Tools::GUIDToString (mediaSubType_in);
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

#if defined (FFMPEG_SUPPORT)
struct _GUID
Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType (enum AVPixelFormat pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType"));

  struct _GUID result = GUID_NULL;

  switch (pixelFormat_in)
  {
    case AV_PIX_FMT_NONE:
      //return MEDIASUBTYPE_None;
      return GUID_NULL;
    case AV_PIX_FMT_YUV420P:
      return MEDIASUBTYPE_YV12;
    case AV_PIX_FMT_YUYV422:
      return MEDIASUBTYPE_YUY2;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
      return MEDIASUBTYPE_RGB24;
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUV410P:
      return GUID_NULL;
    case AV_PIX_FMT_YUV411P:
      return MEDIASUBTYPE_Y411;
    case AV_PIX_FMT_GRAY8:
      return GUID_NULL;
    case AV_PIX_FMT_MONOWHITE:
    case AV_PIX_FMT_MONOBLACK:
      return MEDIASUBTYPE_RGB1;
    case AV_PIX_FMT_PAL8:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUVJ444P:
      return GUID_NULL;
    case AV_PIX_FMT_UYVY422:
      return MEDIASUBTYPE_UYVY;
    case AV_PIX_FMT_UYYVYY411:
      return MEDIASUBTYPE_Y411;
    //case AV_PIX_FMT_BGR8:
    //case AV_PIX_FMT_BGR4:
    //case AV_PIX_FMT_BGR4_BYTE:
    case AV_PIX_FMT_BGR8:
    case AV_PIX_FMT_RGB8:
      return MEDIASUBTYPE_RGB8;
    case AV_PIX_FMT_BGR4:
    case AV_PIX_FMT_BGR4_BYTE:
    case AV_PIX_FMT_RGB4:
    case AV_PIX_FMT_RGB4_BYTE:
      return MEDIASUBTYPE_RGB4;
    case AV_PIX_FMT_NV12:
      return MEDIASUBTYPE_NV12;
    case AV_PIX_FMT_NV21:
      return MEDIASUBTYPE_IMC4;

    case AV_PIX_FMT_ARGB:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_ABGR:
    case AV_PIX_FMT_BGRA:
      return MEDIASUBTYPE_ARGB32;

    case AV_PIX_FMT_GRAY16BE:
    case AV_PIX_FMT_GRAY16LE:
    case AV_PIX_FMT_YUV440P:
    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUVA420P:
    case AV_PIX_FMT_RGB48BE:
    case AV_PIX_FMT_RGB48LE:
      return GUID_NULL;

    case AV_PIX_FMT_RGB565BE:
    case AV_PIX_FMT_RGB565LE:
      return MEDIASUBTYPE_RGB565;
    case AV_PIX_FMT_RGB555BE:
    case AV_PIX_FMT_RGB555LE:
      return MEDIASUBTYPE_RGB555;

    case AV_PIX_FMT_BGR565BE:
    case AV_PIX_FMT_BGR565LE:
      return MEDIASUBTYPE_RGB565;
    case AV_PIX_FMT_BGR555BE:
    case AV_PIX_FMT_BGR555LE:
      return MEDIASUBTYPE_RGB555;

#if FF_API_VAAPI
    case AV_PIX_FMT_VAAPI_MOCO:
    case AV_PIX_FMT_VAAPI_IDCT:
    case AV_PIX_FMT_VAAPI_VLD:
#else
    case AV_PIX_FMT_VAAPI:
#endif
      return GUID_NULL;

    case AV_PIX_FMT_YUV420P16LE:
    case AV_PIX_FMT_YUV420P16BE:
    case AV_PIX_FMT_YUV422P16LE:
    case AV_PIX_FMT_YUV422P16BE:
    case AV_PIX_FMT_YUV444P16LE:
    case AV_PIX_FMT_YUV444P16BE:
    case AV_PIX_FMT_DXVA2_VLD:
      return GUID_NULL;

    case AV_PIX_FMT_RGB444LE:
    case AV_PIX_FMT_RGB444BE:
    case AV_PIX_FMT_BGR444LE:
    case AV_PIX_FMT_BGR444BE:
    case AV_PIX_FMT_YA8:
      return GUID_NULL;

    case AV_PIX_FMT_BGR48BE:
    case AV_PIX_FMT_BGR48LE:
      return GUID_NULL;

    case AV_PIX_FMT_YUV420P9BE:
    case AV_PIX_FMT_YUV420P9LE:
    case AV_PIX_FMT_YUV420P10BE:
    case AV_PIX_FMT_YUV420P10LE:
    case AV_PIX_FMT_YUV422P10BE:
    case AV_PIX_FMT_YUV422P10LE:
    case AV_PIX_FMT_YUV444P9BE:
    case AV_PIX_FMT_YUV444P9LE:
    case AV_PIX_FMT_YUV444P10BE:
    case AV_PIX_FMT_YUV444P10LE:
    case AV_PIX_FMT_YUV422P9BE:
    case AV_PIX_FMT_YUV422P9LE:
    case AV_PIX_FMT_GBRP:
    case AV_PIX_FMT_GBRP9BE:
    case AV_PIX_FMT_GBRP9LE:
    case AV_PIX_FMT_GBRP10BE:
    case AV_PIX_FMT_GBRP10LE:
    case AV_PIX_FMT_GBRP16BE:
    case AV_PIX_FMT_GBRP16LE:
    case AV_PIX_FMT_YUVA422P:
      return GUID_NULL;
    case AV_PIX_FMT_YUVA444P:
      return MEDIASUBTYPE_AYUV;
    case AV_PIX_FMT_YUVA420P9BE:
    case AV_PIX_FMT_YUVA420P9LE:
    case AV_PIX_FMT_YUVA422P9BE:
    case AV_PIX_FMT_YUVA422P9LE:
    case AV_PIX_FMT_YUVA444P9BE:
    case AV_PIX_FMT_YUVA444P9LE:
    case AV_PIX_FMT_YUVA420P10BE:
    case AV_PIX_FMT_YUVA420P10LE:
    case AV_PIX_FMT_YUVA422P10BE:
    case AV_PIX_FMT_YUVA422P10LE:
    case AV_PIX_FMT_YUVA444P10BE:
    case AV_PIX_FMT_YUVA444P10LE:
    case AV_PIX_FMT_YUVA420P16BE:
    case AV_PIX_FMT_YUVA420P16LE:
    case AV_PIX_FMT_YUVA422P16BE:
    case AV_PIX_FMT_YUVA422P16LE:
    case AV_PIX_FMT_YUVA444P16BE:
    case AV_PIX_FMT_YUVA444P16LE:
      return GUID_NULL;

    case AV_PIX_FMT_VDPAU:
      return GUID_NULL;

    case AV_PIX_FMT_XYZ12LE:
    case AV_PIX_FMT_XYZ12BE:
    case AV_PIX_FMT_NV16:
    case AV_PIX_FMT_NV20LE:
    case AV_PIX_FMT_NV20BE:
      return GUID_NULL;

    case AV_PIX_FMT_RGBA64BE:
    case AV_PIX_FMT_RGBA64LE:
    case AV_PIX_FMT_BGRA64BE:
    case AV_PIX_FMT_BGRA64LE:
      return GUID_NULL;

    case AV_PIX_FMT_YVYU422:
      return MEDIASUBTYPE_YVYU;

    case AV_PIX_FMT_YA16BE:
    case AV_PIX_FMT_YA16LE:
      return GUID_NULL;

    case AV_PIX_FMT_GBRAP:
    case AV_PIX_FMT_GBRAP16BE:
    case AV_PIX_FMT_GBRAP16LE:
      return GUID_NULL;

    case AV_PIX_FMT_QSV:
      return GUID_NULL;

    case AV_PIX_FMT_MMAL:
      return GUID_NULL;

    case AV_PIX_FMT_D3D11VA_VLD:
      return GUID_NULL;

    case AV_PIX_FMT_CUDA:
      return GUID_NULL;

    case AV_PIX_FMT_0RGB:
    case AV_PIX_FMT_RGB0:
    case AV_PIX_FMT_0BGR:
    case AV_PIX_FMT_BGR0:
      return GUID_NULL;

    case AV_PIX_FMT_YUV420P12BE:
    case AV_PIX_FMT_YUV420P12LE:
    case AV_PIX_FMT_YUV420P14BE:
    case AV_PIX_FMT_YUV420P14LE:
    case AV_PIX_FMT_YUV422P12BE:
    case AV_PIX_FMT_YUV422P12LE:
    case AV_PIX_FMT_YUV422P14BE:
    case AV_PIX_FMT_YUV422P14LE:
    case AV_PIX_FMT_YUV444P12BE:
    case AV_PIX_FMT_YUV444P12LE:
    case AV_PIX_FMT_YUV444P14BE:
    case AV_PIX_FMT_YUV444P14LE:
    case AV_PIX_FMT_GBRP12BE:
    case AV_PIX_FMT_GBRP12LE:
    case AV_PIX_FMT_GBRP14BE:
    case AV_PIX_FMT_GBRP14LE:
    case AV_PIX_FMT_YUVJ411P:
      return GUID_NULL;

    case AV_PIX_FMT_BAYER_BGGR8:
    case AV_PIX_FMT_BAYER_RGGB8:
    case AV_PIX_FMT_BAYER_GBRG8:
    case AV_PIX_FMT_BAYER_GRBG8:
    case AV_PIX_FMT_BAYER_BGGR16LE:
    case AV_PIX_FMT_BAYER_BGGR16BE:
    case AV_PIX_FMT_BAYER_RGGB16LE:
    case AV_PIX_FMT_BAYER_RGGB16BE:
    case AV_PIX_FMT_BAYER_GBRG16LE:
    case AV_PIX_FMT_BAYER_GBRG16BE:
    case AV_PIX_FMT_BAYER_GRBG16LE:
    case AV_PIX_FMT_BAYER_GRBG16BE:
      return GUID_NULL;

    case AV_PIX_FMT_XVMC:
      return GUID_NULL;

    case AV_PIX_FMT_YUV440P10LE:
    case AV_PIX_FMT_YUV440P10BE:
    case AV_PIX_FMT_YUV440P12LE:
    case AV_PIX_FMT_YUV440P12BE:
    case AV_PIX_FMT_AYUV64LE:
    case AV_PIX_FMT_AYUV64BE:
      return GUID_NULL;

    case AV_PIX_FMT_VIDEOTOOLBOX:
      return GUID_NULL;

    case AV_PIX_FMT_P010LE:
    case AV_PIX_FMT_P010BE:
      return GUID_NULL;

    case AV_PIX_FMT_GBRAP12BE:
    case AV_PIX_FMT_GBRAP12LE:
      return GUID_NULL;

    case AV_PIX_FMT_GBRAP10BE:
    case AV_PIX_FMT_GBRAP10LE:
      return GUID_NULL;

    case AV_PIX_FMT_MEDIACODEC:
      return GUID_NULL;

    case AV_PIX_FMT_GRAY12BE:
    case AV_PIX_FMT_GRAY12LE:
    case AV_PIX_FMT_GRAY10BE:
    case AV_PIX_FMT_GRAY10LE:
      return GUID_NULL;

    case AV_PIX_FMT_P016LE:
    case AV_PIX_FMT_P016BE:
      return MEDIASUBTYPE_IMC1;

    case AV_PIX_FMT_D3D11:
      return GUID_NULL;

    case AV_PIX_FMT_GRAY9BE:
    case AV_PIX_FMT_GRAY9LE:
      return GUID_NULL;

    case AV_PIX_FMT_GBRPF32BE:
    case AV_PIX_FMT_GBRPF32LE:
    case AV_PIX_FMT_GBRAPF32BE:
    case AV_PIX_FMT_GBRAPF32LE:
      return GUID_NULL;

    case AV_PIX_FMT_DRM_PRIME:
      return GUID_NULL;

    case AV_PIX_FMT_OPENCL:
      return GUID_NULL;

    case AV_PIX_FMT_GRAY14BE:
    case AV_PIX_FMT_GRAY14LE:
      return GUID_NULL;

    case AV_PIX_FMT_GRAYF32BE:
    case AV_PIX_FMT_GRAYF32LE:
      return GUID_NULL;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %d \"%s\"), aborting\n"),
                  pixelFormat_in, ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (pixelFormat_in).c_str ())));
      break;
    }
  } // end SWITCH

  return result;
}
#endif // FFMPEG_SUPPORT
#else
#if defined (FFMPEG_SUPPORT)
unsigned int
Stream_MediaFramework_Tools::ffmpegFormatToBitDepth (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::ffmpegFormatToBitDepth"));

  switch (format_in)
  {
    case AV_PIX_FMT_NONE:
      return 0;
    case AV_PIX_FMT_YUV420P:
      return 12;
    case AV_PIX_FMT_YUYV422:
      return 16;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
      return 24;
    case AV_PIX_FMT_YUV422P:
      return 16;
    case AV_PIX_FMT_YUV444P:
      return 24;
    case AV_PIX_FMT_YUV410P:
      return 9;
    case AV_PIX_FMT_YUV411P:
      return 12;
    case AV_PIX_FMT_GRAY8:
      return 8;
    case AV_PIX_FMT_MONOWHITE:
    case AV_PIX_FMT_MONOBLACK:
      return 1;
    case AV_PIX_FMT_PAL8:
      return 8;
    case AV_PIX_FMT_YUVJ420P:
      return 12;
    case AV_PIX_FMT_YUVJ422P:
      return 16;
    case AV_PIX_FMT_YUVJ444P:
      return 24;
    case AV_PIX_FMT_UYVY422:
      return 16;
    case AV_PIX_FMT_UYYVYY411:
      return 12;
    case AV_PIX_FMT_BGR8:
      return 8;
    case AV_PIX_FMT_BGR4:
      return 4;
    case AV_PIX_FMT_BGR4_BYTE:
    case AV_PIX_FMT_RGB8:
      return 8;
    case AV_PIX_FMT_RGB4:
      return 4;
    case AV_PIX_FMT_RGB4_BYTE:
      return 8;
    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
      return 12;
    case AV_PIX_FMT_ARGB:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_ABGR:
    case AV_PIX_FMT_BGRA:
      return 32;
    case AV_PIX_FMT_GRAY16BE:
    case AV_PIX_FMT_GRAY16LE:
      return 16;
    case AV_PIX_FMT_YUV440P:
    case AV_PIX_FMT_YUVJ440P:
      return 0;
    case AV_PIX_FMT_YUVA420P:
      return 20;
    case AV_PIX_FMT_RGB48BE:
    case AV_PIX_FMT_RGB48LE:
      return 48;
    case AV_PIX_FMT_RGB565BE:
    case AV_PIX_FMT_RGB565LE:
    case AV_PIX_FMT_RGB555BE:
    case AV_PIX_FMT_RGB555LE:
    case AV_PIX_FMT_BGR565BE:
    case AV_PIX_FMT_BGR565LE:
    case AV_PIX_FMT_BGR555BE:
    case AV_PIX_FMT_BGR555LE:
      return 16;
#if FF_API_VAAPI
    case AV_PIX_FMT_VAAPI_MOCO:
    case AV_PIX_FMT_VAAPI_IDCT:
    case AV_PIX_FMT_VAAPI_VLD:
#else
    case AV_PIX_FMT_VAAPI:
#endif
      return 0;
    case AV_PIX_FMT_YUV420P16LE:
    case AV_PIX_FMT_YUV420P16BE:
      return 24;
    case AV_PIX_FMT_YUV422P16LE:
    case AV_PIX_FMT_YUV422P16BE:
      return 32;
    case AV_PIX_FMT_YUV444P16LE:
    case AV_PIX_FMT_YUV444P16BE:
      return 48;
    case AV_PIX_FMT_DXVA2_VLD:
      return 0;
    case AV_PIX_FMT_RGB444LE:
    case AV_PIX_FMT_RGB444BE:
    case AV_PIX_FMT_BGR444LE:
    case AV_PIX_FMT_BGR444BE:
      return 16;
    case AV_PIX_FMT_YA8:
//    case AV_PIX_FMT_Y400A:
//    case AV_PIX_FMT_GRAY8A:
      return 8;
    case AV_PIX_FMT_BGR48BE:
    case AV_PIX_FMT_BGR48LE:
      return 48;
    case AV_PIX_FMT_YUV420P9BE:
    case AV_PIX_FMT_YUV420P9LE:
      return 12;
    case AV_PIX_FMT_YUV420P10BE:
    case AV_PIX_FMT_YUV420P10LE:
      return 15;
    case AV_PIX_FMT_YUV422P10BE:
    case AV_PIX_FMT_YUV422P10LE:
      return 20;
    case AV_PIX_FMT_YUV444P9BE:
    case AV_PIX_FMT_YUV444P9LE:
      return 27;
    case AV_PIX_FMT_YUV444P10BE:
    case AV_PIX_FMT_YUV444P10LE:
      return 30;
    case AV_PIX_FMT_YUV422P9BE:
    case AV_PIX_FMT_YUV422P9LE:
      return 18;
    case AV_PIX_FMT_GBRP:
//    case AV_PIX_FMT_GBR24P:
      return 24;
    case AV_PIX_FMT_GBRP9BE:
    case AV_PIX_FMT_GBRP9LE:
      return 27;
    case AV_PIX_FMT_GBRP10BE:
    case AV_PIX_FMT_GBRP10LE:
      return 30;
    case AV_PIX_FMT_GBRP16BE:
    case AV_PIX_FMT_GBRP16LE:
      return 48;
    case AV_PIX_FMT_YUVA422P:
      return 24;
    case AV_PIX_FMT_YUVA444P:
      return 32;
    case AV_PIX_FMT_YUVA420P9BE:
    case AV_PIX_FMT_YUVA420P9LE:
      return 24;
    case AV_PIX_FMT_YUVA422P9BE:
    case AV_PIX_FMT_YUVA422P9LE:
      return 27;
    case AV_PIX_FMT_YUVA444P9BE:
    case AV_PIX_FMT_YUVA444P9LE:
      return 36;
    case AV_PIX_FMT_YUVA420P10BE:
    case AV_PIX_FMT_YUVA420P10LE:
      return 25;
    case AV_PIX_FMT_YUVA422P10BE:
    case AV_PIX_FMT_YUVA422P10LE:
      return 30;
    case AV_PIX_FMT_YUVA444P10BE:
    case AV_PIX_FMT_YUVA444P10LE:
      return 40;
    case AV_PIX_FMT_YUVA420P16BE:
    case AV_PIX_FMT_YUVA420P16LE:
      return 40;
    case AV_PIX_FMT_YUVA422P16BE:
    case AV_PIX_FMT_YUVA422P16LE:
      return 48;
    case AV_PIX_FMT_YUVA444P16BE:
    case AV_PIX_FMT_YUVA444P16LE:
      return 64;
    case AV_PIX_FMT_VDPAU:
      return 0;
    case AV_PIX_FMT_XYZ12LE:
    case AV_PIX_FMT_XYZ12BE:
      return 36;
    case AV_PIX_FMT_NV16:
      return 16;
    case AV_PIX_FMT_NV20LE:
    case AV_PIX_FMT_NV20BE:
      return 20;
    case AV_PIX_FMT_RGBA64BE:
    case AV_PIX_FMT_RGBA64LE:
    case AV_PIX_FMT_BGRA64BE:
    case AV_PIX_FMT_BGRA64LE:
      return 64;
    case AV_PIX_FMT_YVYU422:
      return 16;
    case AV_PIX_FMT_YA16BE:
    case AV_PIX_FMT_YA16LE:
      return 16;
    case AV_PIX_FMT_GBRAP:
      return 32;
    case AV_PIX_FMT_GBRAP16BE:
    case AV_PIX_FMT_GBRAP16LE:
      return 64;
    case AV_PIX_FMT_QSV:
    case AV_PIX_FMT_MMAL:
    case AV_PIX_FMT_D3D11VA_VLD:
    case AV_PIX_FMT_CUDA:
      return 0;
    case AV_PIX_FMT_0RGB:
    case AV_PIX_FMT_RGB0:
    case AV_PIX_FMT_0BGR:
    case AV_PIX_FMT_BGR0:
      return 32;
    case AV_PIX_FMT_YUV420P12BE:
    case AV_PIX_FMT_YUV420P12LE:
      return 18;
    case AV_PIX_FMT_YUV420P14BE:
    case AV_PIX_FMT_YUV420P14LE:
      return 21;
    case AV_PIX_FMT_YUV422P12BE:
    case AV_PIX_FMT_YUV422P12LE:
      return 24;
    case AV_PIX_FMT_YUV422P14BE:
    case AV_PIX_FMT_YUV422P14LE:
      return 28;
    case AV_PIX_FMT_YUV444P12BE:
    case AV_PIX_FMT_YUV444P12LE:
      return 36;
    case AV_PIX_FMT_YUV444P14BE:
    case AV_PIX_FMT_YUV444P14LE:
      return 42;
    case AV_PIX_FMT_GBRP12BE:
    case AV_PIX_FMT_GBRP12LE:
      return 36;
    case AV_PIX_FMT_GBRP14BE:
    case AV_PIX_FMT_GBRP14LE:
      return 42;
    case AV_PIX_FMT_YUVJ411P:
      return 12;
    case AV_PIX_FMT_BAYER_BGGR8:
    case AV_PIX_FMT_BAYER_RGGB8:
    case AV_PIX_FMT_BAYER_GBRG8:
    case AV_PIX_FMT_BAYER_GRBG8:
      return 32;
    case AV_PIX_FMT_BAYER_BGGR16LE:
    case AV_PIX_FMT_BAYER_BGGR16BE:
    case AV_PIX_FMT_BAYER_RGGB16LE:
    case AV_PIX_FMT_BAYER_RGGB16BE:
    case AV_PIX_FMT_BAYER_GBRG16LE:
    case AV_PIX_FMT_BAYER_GBRG16BE:
    case AV_PIX_FMT_BAYER_GRBG16LE:
    case AV_PIX_FMT_BAYER_GRBG16BE:
      return 64;
    case AV_PIX_FMT_XVMC:
      return 0;
    case AV_PIX_FMT_YUV440P10LE:
    case AV_PIX_FMT_YUV440P10BE:
      return 20;
    case AV_PIX_FMT_YUV440P12LE:
    case AV_PIX_FMT_YUV440P12BE:
      return 24;
    case AV_PIX_FMT_AYUV64LE:
    case AV_PIX_FMT_AYUV64BE:
      return 64;
    case AV_PIX_FMT_VIDEOTOOLBOX:
      return 0;
    case AV_PIX_FMT_P010LE:
    case AV_PIX_FMT_P010BE:
      return 12;
    case AV_PIX_FMT_GBRAP12BE:
    case AV_PIX_FMT_GBRAP12LE:
      return 48;
    case AV_PIX_FMT_GBRAP10BE:
    case AV_PIX_FMT_GBRAP10LE:
      return 40;
    case AV_PIX_FMT_MEDIACODEC:
      return 0;
    case AV_PIX_FMT_GRAY12BE:
    case AV_PIX_FMT_GRAY12LE:
      return 12;
    case AV_PIX_FMT_GRAY10BE:
    case AV_PIX_FMT_GRAY10LE:
      return 10;
    case AV_PIX_FMT_P016LE:
    case AV_PIX_FMT_P016BE:
      return 16;
    case AV_PIX_FMT_D3D11:
      return 0;
    case AV_PIX_FMT_GRAY9BE:
    case AV_PIX_FMT_GRAY9LE:
      return 9;
    case AV_PIX_FMT_GBRPF32BE:
    case AV_PIX_FMT_GBRPF32LE:
      return 96;
    case AV_PIX_FMT_GBRAPF32BE:
    case AV_PIX_FMT_GBRAPF32LE:
      return 128;
    case AV_PIX_FMT_DRM_PRIME:
      return 0;
    case AV_PIX_FMT_OPENCL:
      return 0;
    case AV_PIX_FMT_GRAY14BE:
    case AV_PIX_FMT_GRAY14LE:
      return 14;
    case AV_PIX_FMT_GRAYF32BE:
    case AV_PIX_FMT_GRAYF32LE:
      return 32;
    case AV_PIX_FMT_YUVA422P12BE:
    case AV_PIX_FMT_YUVA422P12LE:
      return 24;
    case AV_PIX_FMT_YUVA444P12BE:
    case AV_PIX_FMT_YUVA444P12LE:
      return 36;
    case AV_PIX_FMT_NV24:
    case AV_PIX_FMT_NV42:
      return 24;
//    case AV_PIX_FMT_VULKAN:
//      return 0;
//    case AV_PIX_FMT_Y210BE:
//    case AV_PIX_FMT_Y210LE:
//      return 20;
//    case AV_PIX_FMT_X2RGB10LE:
//    case AV_PIX_FMT_X2RGB10BE:
//      return 30;
    case AV_PIX_FMT_NB:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ffmpeg pixel format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return 0;
}
#endif // FFMPEG_SUPPORT

unsigned int
Stream_MediaFramework_Tools::v4lFormatToBitDepth (__u32 format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::v4lFormatToBitDepth"));

  switch (format_in)
  {
    case V4L2_PIX_FMT_RGB332:
      return 8;
    case V4L2_PIX_FMT_RGB444:
      return 12;
    case V4L2_PIX_FMT_ARGB444:
    case V4L2_PIX_FMT_XRGB444:
    case V4L2_PIX_FMT_RGBA444:
    case V4L2_PIX_FMT_RGBX444:
    case V4L2_PIX_FMT_ABGR444:
    case V4L2_PIX_FMT_XBGR444:
    case V4L2_PIX_FMT_BGRA444:
    case V4L2_PIX_FMT_BGRX444:
      return 12;
    case V4L2_PIX_FMT_RGB555:
      return 15;
    case V4L2_PIX_FMT_ARGB555:
    case V4L2_PIX_FMT_XRGB555:
    case V4L2_PIX_FMT_RGBA555:
    case V4L2_PIX_FMT_RGBX555:
    case V4L2_PIX_FMT_ABGR555:
    case V4L2_PIX_FMT_XBGR555:
    case V4L2_PIX_FMT_BGRA555:
    case V4L2_PIX_FMT_BGRX555:
      return 15;
    case V4L2_PIX_FMT_RGB565:
      return 16;
    case V4L2_PIX_FMT_RGB555X:
    case V4L2_PIX_FMT_ARGB555X:
    case V4L2_PIX_FMT_XRGB555X:
      return 15;
    case V4L2_PIX_FMT_RGB565X:
      return 16;
    case V4L2_PIX_FMT_BGR666:
      return 18;
    case V4L2_PIX_FMT_BGR24:
    case V4L2_PIX_FMT_RGB24:
      return 24;
    case V4L2_PIX_FMT_BGR32:
    case V4L2_PIX_FMT_ABGR32:
    case V4L2_PIX_FMT_XBGR32:
    case V4L2_PIX_FMT_BGRA32:
    case V4L2_PIX_FMT_BGRX32:
    case V4L2_PIX_FMT_RGB32:
    case V4L2_PIX_FMT_RGBA32:
    case V4L2_PIX_FMT_RGBX32:
    case V4L2_PIX_FMT_ARGB32:
    case V4L2_PIX_FMT_XRGB32:
      return 32;
    case V4L2_PIX_FMT_GREY:
      return 8;
    case V4L2_PIX_FMT_Y4:
      return 4;
    case V4L2_PIX_FMT_Y6:
      return 6;
    case V4L2_PIX_FMT_Y10:
      return 10;
    case V4L2_PIX_FMT_Y12:
      return 12;
    //case V4L2_PIX_FMT_Y14:
    //  return 14;
    case V4L2_PIX_FMT_Y16:
    case V4L2_PIX_FMT_Y16_BE:
      return 16;
    case V4L2_PIX_FMT_Y10BPACK:
    case V4L2_PIX_FMT_Y10P:
      return 10;
    case V4L2_PIX_FMT_PAL8:
      return 8;
    case V4L2_PIX_FMT_UV8:
      return 8;
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YYUV:
    case V4L2_PIX_FMT_YVYU:
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_VYUY:
      return 16;
    case V4L2_PIX_FMT_Y41P:
      return 12;
    case V4L2_PIX_FMT_YUV444:
    case V4L2_PIX_FMT_YUV555:
    case V4L2_PIX_FMT_YUV565:
      return 16;
    //case V4L2_PIX_FMT_YUV24:
    //  return 24;
    case V4L2_PIX_FMT_YUV32:
      return 32;
    case V4L2_PIX_FMT_AYUV32:
    case V4L2_PIX_FMT_XYUV32:
    case V4L2_PIX_FMT_VUYA32:
    case V4L2_PIX_FMT_VUYX32:
      return 32;
    case V4L2_PIX_FMT_M420:
      return 12;
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
      return 12;
    case V4L2_PIX_FMT_NV16:
    case V4L2_PIX_FMT_NV61:
      return 16;
    case V4L2_PIX_FMT_NV24:
    case V4L2_PIX_FMT_NV42:
      return 24;
    case V4L2_PIX_FMT_HM12:
      return 12;
    case V4L2_PIX_FMT_NV12M:
    case V4L2_PIX_FMT_NV21M:
      return 12;
    case V4L2_PIX_FMT_NV16M:
    case V4L2_PIX_FMT_NV61M:
      return 16;
    case V4L2_PIX_FMT_NV12MT:
    case V4L2_PIX_FMT_NV12MT_16X16:
      return 12;
    case V4L2_PIX_FMT_YUV410:
    case V4L2_PIX_FMT_YVU410:
      return 9;
    case V4L2_PIX_FMT_YUV411P:
      return 12;
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420: // 'YV12'
      return 12;
    case V4L2_PIX_FMT_YUV422P:
      return 16;
    case V4L2_PIX_FMT_YUV420M:
    case V4L2_PIX_FMT_YVU420M:
      return 12;
    case V4L2_PIX_FMT_YUV422M:
    case V4L2_PIX_FMT_YVU422M:
      return 16;
    case V4L2_PIX_FMT_YUV444M:
    case V4L2_PIX_FMT_YVU444M:
      return 16;
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SRGGB8:
      return 32;
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SRGGB10:
      return 40;
    case V4L2_PIX_FMT_SBGGR10P:
    case V4L2_PIX_FMT_SGBRG10P:
    case V4L2_PIX_FMT_SGRBG10P:
    case V4L2_PIX_FMT_SRGGB10P:
      return 40;
    case V4L2_PIX_FMT_SBGGR10ALAW8:
    case V4L2_PIX_FMT_SGBRG10ALAW8:
    case V4L2_PIX_FMT_SGRBG10ALAW8:
    case V4L2_PIX_FMT_SRGGB10ALAW8:
      return 40;
    case V4L2_PIX_FMT_SBGGR10DPCM8:
    case V4L2_PIX_FMT_SGBRG10DPCM8:
    case V4L2_PIX_FMT_SGRBG10DPCM8:
    case V4L2_PIX_FMT_SRGGB10DPCM8:
      return 40;
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SRGGB12:
      return 48;
    case V4L2_PIX_FMT_SBGGR12P:
    case V4L2_PIX_FMT_SGBRG12P:
    case V4L2_PIX_FMT_SGRBG12P:
    case V4L2_PIX_FMT_SRGGB12P:
      return 48;
    //case V4L2_PIX_FMT_SBGGR14:
    //case V4L2_PIX_FMT_SGBRG14:
    //case V4L2_PIX_FMT_SGRBG14:
    //case V4L2_PIX_FMT_SRGGB14:
    //  return 56;
    case V4L2_PIX_FMT_SBGGR14P:
    case V4L2_PIX_FMT_SGBRG14P:
    case V4L2_PIX_FMT_SGRBG14P:
    case V4L2_PIX_FMT_SRGGB14P:
      return 56;
    case V4L2_PIX_FMT_SBGGR16:
    case V4L2_PIX_FMT_SGBRG16:
    case V4L2_PIX_FMT_SGRBG16:
    case V4L2_PIX_FMT_SRGGB16:
      return 64;
    case V4L2_PIX_FMT_HSV24:
      return 24;
    case V4L2_PIX_FMT_HSV32:
      return 32;
    case V4L2_PIX_FMT_MJPEG:
      return 16;
    case V4L2_PIX_FMT_JPEG:
      return 16;
    case V4L2_PIX_FMT_DV:
    case V4L2_PIX_FMT_MPEG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H264_NO_SC:
    case V4L2_PIX_FMT_H264_MVC:
    case V4L2_PIX_FMT_H263:
    case V4L2_PIX_FMT_MPEG1:
    case V4L2_PIX_FMT_MPEG2:
    case V4L2_PIX_FMT_MPEG2_SLICE:
    case V4L2_PIX_FMT_MPEG4:
    case V4L2_PIX_FMT_XVID:
    case V4L2_PIX_FMT_VC1_ANNEX_G:
    case V4L2_PIX_FMT_VC1_ANNEX_L:
    case V4L2_PIX_FMT_VP8:
    //case V4L2_PIX_FMT_VP8_FRAME:
    case V4L2_PIX_FMT_VP9:
    case V4L2_PIX_FMT_HEVC:
    case V4L2_PIX_FMT_FWHT:
    case V4L2_PIX_FMT_FWHT_STATELESS:
    //case V4L2_PIX_FMT_H264_SLICE:
      return 0;
    case V4L2_PIX_FMT_CPIA1:
    case V4L2_PIX_FMT_WNVA:
    case V4L2_PIX_FMT_SN9C10X:
    case V4L2_PIX_FMT_SN9C20X_I420:
    case V4L2_PIX_FMT_PWC1:
    case V4L2_PIX_FMT_PWC2:
    case V4L2_PIX_FMT_ET61X251:
    case V4L2_PIX_FMT_SPCA501:
    case V4L2_PIX_FMT_SPCA505:
    case V4L2_PIX_FMT_SPCA508:
    case V4L2_PIX_FMT_SPCA561:
    case V4L2_PIX_FMT_PAC207:
    case V4L2_PIX_FMT_MR97310A:
    case V4L2_PIX_FMT_JL2005BCD:
    case V4L2_PIX_FMT_SN9C2028:
    case V4L2_PIX_FMT_SQ905C:
    case V4L2_PIX_FMT_PJPG:
    case V4L2_PIX_FMT_OV511:
    case V4L2_PIX_FMT_OV518:
    case V4L2_PIX_FMT_STV0680:
    case V4L2_PIX_FMT_TM6000:
    case V4L2_PIX_FMT_CIT_YYVYUY:
    case V4L2_PIX_FMT_KONICA420:
    case V4L2_PIX_FMT_JPGL:
    case V4L2_PIX_FMT_SE401:
    case V4L2_PIX_FMT_S5C_UYVY_JPG:
    case V4L2_PIX_FMT_Y8I:
    case V4L2_PIX_FMT_Y12I:
    case V4L2_PIX_FMT_Z16:
    case V4L2_PIX_FMT_MT21C:
    case V4L2_PIX_FMT_INZI:
    case V4L2_PIX_FMT_SUNXI_TILED_NV12:
    case V4L2_PIX_FMT_CNF4:
    case V4L2_PIX_FMT_HI240:
      return 0;
    case V4L2_PIX_FMT_IPU3_SBGGR10:
    case V4L2_PIX_FMT_IPU3_SGBRG10:
    case V4L2_PIX_FMT_IPU3_SGRBG10:
    case V4L2_PIX_FMT_IPU3_SRGGB10:
      return 40;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown v4l2 pixel format (was: \"%s\" [%d]), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::v4lFormatToString (format_in).c_str ()), format_in));
      break;
    }
  } // end SWITCH

  return 0;
}

bool
Stream_MediaFramework_Tools::isRGB (__u32 format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::isRGB"));

  switch (format_in)
  {
    case V4L2_PIX_FMT_RGB332:
    case V4L2_PIX_FMT_RGB444:
    case V4L2_PIX_FMT_ARGB444:
    case V4L2_PIX_FMT_XRGB444:
    case V4L2_PIX_FMT_RGBA444:
    case V4L2_PIX_FMT_RGBX444:
    case V4L2_PIX_FMT_ABGR444:
    case V4L2_PIX_FMT_XBGR444:
    case V4L2_PIX_FMT_BGRA444:
    case V4L2_PIX_FMT_BGRX444:
    case V4L2_PIX_FMT_RGB555:
    case V4L2_PIX_FMT_ARGB555:
    case V4L2_PIX_FMT_XRGB555:
    case V4L2_PIX_FMT_RGBA555:
    case V4L2_PIX_FMT_RGBX555:
    case V4L2_PIX_FMT_ABGR555:
    case V4L2_PIX_FMT_XBGR555:
    case V4L2_PIX_FMT_BGRA555:
    case V4L2_PIX_FMT_BGRX555:
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_RGB555X:
    case V4L2_PIX_FMT_ARGB555X:
    case V4L2_PIX_FMT_XRGB555X:
    case V4L2_PIX_FMT_RGB565X:
    case V4L2_PIX_FMT_BGR666:
    case V4L2_PIX_FMT_BGR24:
    case V4L2_PIX_FMT_RGB24:
    case V4L2_PIX_FMT_BGR32:
    case V4L2_PIX_FMT_ABGR32:
    case V4L2_PIX_FMT_XBGR32:
    case V4L2_PIX_FMT_BGRA32:
    case V4L2_PIX_FMT_BGRX32:
    case V4L2_PIX_FMT_RGB32:
    case V4L2_PIX_FMT_RGBA32:
    case V4L2_PIX_FMT_RGBX32:
    case V4L2_PIX_FMT_ARGB32:
    case V4L2_PIX_FMT_XRGB32:
      return true;
    default:
      break;
  } // end SWITCH

  return false;
}

#if defined (FFMPEG_SUPPORT)
__u32
Stream_MediaFramework_Tools::ffmpegFormatToV4lFormat (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::ffmpegFormatToV4lFormat"));

  switch (format_in)
  {
    case AV_PIX_FMT_YUV420P:
      return V4L2_PIX_FMT_YUV420;
    case AV_PIX_FMT_YUYV422:
      return V4L2_PIX_FMT_YUYV;
    case AV_PIX_FMT_RGB24:
      return V4L2_PIX_FMT_RGB24;
    case AV_PIX_FMT_BGR24:
      return V4L2_PIX_FMT_BGR24;
    case AV_PIX_FMT_YUV422P:
      return V4L2_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUV444P:
      return V4L2_PIX_FMT_YUV444;
    case AV_PIX_FMT_YUV410P:
      return V4L2_PIX_FMT_YUV410;
    case AV_PIX_FMT_YUV411P:
      return V4L2_PIX_FMT_YUV411P;
    case AV_PIX_FMT_GRAY8:
      return V4L2_PIX_FMT_GREY;
//    case AV_PIX_FMT_MONOWHITE:
//    case AV_PIX_FMT_MONOBLACK:
    case AV_PIX_FMT_PAL8:
      return V4L2_PIX_FMT_PAL8;
//    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVJ422P:
      // *TODO*: libav doesn't specify a pixel format for MJPEG (it is a codec)
      return V4L2_PIX_FMT_MJPEG;
//    case AV_PIX_FMT_YUVJ444P:
//    case AV_PIX_FMT_XVMC_MPEG2_MC:
//    case AV_PIX_FMT_XVMC_MPEG2_IDCT:
//    case AV_PIX_FMT_XVMC:
    case AV_PIX_FMT_UYVY422:
      return V4L2_PIX_FMT_UYVY;
//    case AV_PIX_FMT_UYYVYY411:
//    case AV_PIX_FMT_BGR8:
//    case AV_PIX_FMT_BGR4:
//    case AV_PIX_FMT_BGR4_BYTE:
//    case AV_PIX_FMT_RGB8:
//    case AV_PIX_FMT_RGB4:
//    case AV_PIX_FMT_RGB4_BYTE:
    case AV_PIX_FMT_NV12:
      return V4L2_PIX_FMT_NV12;
    case AV_PIX_FMT_NV21:
      return V4L2_PIX_FMT_NV21;
    case AV_PIX_FMT_ARGB:
      return V4L2_PIX_FMT_ARGB32;
    case AV_PIX_FMT_RGBA:
      return V4L2_PIX_FMT_RGB32;
    case AV_PIX_FMT_ABGR:
      return V4L2_PIX_FMT_ABGR32;
    case AV_PIX_FMT_BGRA:
      return V4L2_PIX_FMT_BGR32;
    case AV_PIX_FMT_GRAY16BE:
      return V4L2_PIX_FMT_Y16_BE;
    case AV_PIX_FMT_GRAY16LE:
      return V4L2_PIX_FMT_Y16;
//    case AV_PIX_FMT_YUV440P:
//    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUVA420P:
      return V4L2_PIX_FMT_YUV420;
//    case AV_PIX_FMT_VDPAU_H264:
//    case AV_PIX_FMT_VDPAU_MPEG1:
//    case AV_PIX_FMT_VDPAU_MPEG2:
//    case AV_PIX_FMT_VDPAU_WMV3:
//    case AV_PIX_FMT_VDPAU_VC1:
//     case AV_PIX_FMT_RGB48BE:
//     case AV_PIX_FMT_RGB48LE:
     case AV_PIX_FMT_RGB565BE:
      return V4L2_PIX_FMT_RGB565X;
    case AV_PIX_FMT_RGB565LE:
     return V4L2_PIX_FMT_RGB565;
    case AV_PIX_FMT_RGB555BE:
     return V4L2_PIX_FMT_RGB555X;
    case AV_PIX_FMT_RGB555LE:
     return V4L2_PIX_FMT_RGB555;
//    case AV_PIX_FMT_BGR565BE:
//    case AV_PIX_FMT_BGR565LE:
//    case AV_PIX_FMT_BGR555BE:
//    case AV_PIX_FMT_BGR555LE:
//    case AV_PIX_FMT_VAAPI_MOCO:
//    case AV_PIX_FMT_VAAPI_IDCT:
//    case AV_PIX_FMT_VAAPI_VLD:
//    case AV_PIX_FMT_VAAPI:
//    case AV_PIX_FMT_YUV420P16LE:
//    case AV_PIX_FMT_YUV420P16BE:
//    case AV_PIX_FMT_YUV422P16LE:
//    case AV_PIX_FMT_YUV422P16BE:
//    case AV_PIX_FMT_YUV444P16LE:
//    case AV_PIX_FMT_YUV444P16BE:
//    case AV_PIX_FMT_VDPAU_MPEG4:
//    case AV_PIX_FMT_DXVA2_VLD:
    case AV_PIX_FMT_RGB444LE:
    case AV_PIX_FMT_RGB444BE:
      return V4L2_PIX_FMT_RGB444;
//    case AV_PIX_FMT_BGR444LE:
//    case AV_PIX_FMT_BGR444BE:
//    case AV_PIX_FMT_YA8:
//    case AV_PIX_FMT_Y400A:
//    case AV_PIX_FMT_GRAY8A:
//    case AV_PIX_FMT_BGR48BE:
//    case AV_PIX_FMT_BGR48LE:
//    case AV_PIX_FMT_YUV420P9BE:
//    case AV_PIX_FMT_YUV420P9LE:
//    case AV_PIX_FMT_YUV420P10BE:
//    case AV_PIX_FMT_YUV420P10LE:
//    case AV_PIX_FMT_YUV422P10BE:
//    case AV_PIX_FMT_YUV422P10LE:
//    case AV_PIX_FMT_YUV444P9BE:
//    case AV_PIX_FMT_YUV444P9LE:
//    case AV_PIX_FMT_YUV444P10BE:
//    case AV_PIX_FMT_YUV444P10LE:
//    case AV_PIX_FMT_YUV422P9BE:
//    case AV_PIX_FMT_YUV422P9LE:
//    case AV_PIX_FMT_VDA_VLD:
//    case AV_PIX_FMT_GBRP:
//    case AV_PIX_FMT_GBR24P:
//    case AV_PIX_FMT_GBRP9BE:
//    case AV_PIX_FMT_GBRP9LE:
//    case AV_PIX_FMT_GBRP10BE:
//    case AV_PIX_FMT_GBRP10LE:
//    case AV_PIX_FMT_GBRP16BE:
//    case AV_PIX_FMT_GBRP16LE:
//    case AV_PIX_FMT_YUVA422P:
    case AV_PIX_FMT_YUVA444P:
      return V4L2_PIX_FMT_AYUV32;
//    case AV_PIX_FMT_YUVA420P9BE:
//    case AV_PIX_FMT_YUVA420P9LE:
//    case AV_PIX_FMT_YUVA422P9BE:
//    case AV_PIX_FMT_YUVA422P9LE:
//    case AV_PIX_FMT_YUVA444P9BE:
//    case AV_PIX_FMT_YUVA444P9LE:
//    case AV_PIX_FMT_YUVA420P10BE:
//    case AV_PIX_FMT_YUVA420P10LE:
//    case AV_PIX_FMT_YUVA422P10BE:
//    case AV_PIX_FMT_YUVA422P10LE:
//    case AV_PIX_FMT_YUVA444P10BE:
//    case AV_PIX_FMT_YUVA444P10LE:
//    case AV_PIX_FMT_YUVA420P16BE:
//    case AV_PIX_FMT_YUVA420P16LE:
//    case AV_PIX_FMT_YUVA422P16BE:
//    case AV_PIX_FMT_YUVA422P16LE:
//    case AV_PIX_FMT_YUVA444P16BE:
//    case AV_PIX_FMT_YUVA444P16LE:
//    case AV_PIX_FMT_VDPAU:
//    case AV_PIX_FMT_XYZ12LE:
//    case AV_PIX_FMT_XYZ12BE:
    case AV_PIX_FMT_NV16:
      return V4L2_PIX_FMT_NV16;
//    case AV_PIX_FMT_NV20LE:
//    case AV_PIX_FMT_NV20BE:
//    case AV_PIX_FMT_RGBA64BE:
//    case AV_PIX_FMT_RGBA64LE:
//    case AV_PIX_FMT_BGRA64BE:
//    case AV_PIX_FMT_BGRA64LE:
    case AV_PIX_FMT_YVYU422:
      return V4L2_PIX_FMT_YVYU;
//    case AV_PIX_FMT_VDA:
//    case AV_PIX_FMT_YA16BE:
//    case AV_PIX_FMT_YA16LE:
//    case AV_PIX_FMT_GBRAP:
//    case AV_PIX_FMT_GBRAP16BE:
//    case AV_PIX_FMT_GBRAP16LE:
//    case AV_PIX_FMT_QSV:
//    case AV_PIX_FMT_MMAL:
//    case AV_PIX_FMT_D3D11VA_VLD:
//    case AV_PIX_FMT_CUDA:
//    case AV_PIX_FMT_0RGB:
//    case AV_PIX_FMT_RGB0:
//    case AV_PIX_FMT_0BGR:
//    case AV_PIX_FMT_BGR0:
//    case AV_PIX_FMT_YUV420P12BE:
//    case AV_PIX_FMT_YUV420P12LE:
//    case AV_PIX_FMT_YUV420P14BE:
//    case AV_PIX_FMT_YUV420P14LE:
//    case AV_PIX_FMT_YUV422P12BE:
//    case AV_PIX_FMT_YUV422P12LE:
//    case AV_PIX_FMT_YUV422P14BE:
//    case AV_PIX_FMT_YUV422P14LE:
//    case AV_PIX_FMT_YUV444P12BE:
//    case AV_PIX_FMT_YUV444P12LE:
//    case AV_PIX_FMT_YUV444P14BE:
//    case AV_PIX_FMT_YUV444P14LE:
//    case AV_PIX_FMT_GBRP12BE:
//    case AV_PIX_FMT_GBRP12LE:
//    case AV_PIX_FMT_GBRP14BE:
//    case AV_PIX_FMT_GBRP14LE:
//    case AV_PIX_FMT_YUVJ411P:
    case AV_PIX_FMT_BAYER_BGGR8:
      return V4L2_PIX_FMT_SBGGR8;
    case AV_PIX_FMT_BAYER_RGGB8:
      return V4L2_PIX_FMT_SRGGB8;
    case AV_PIX_FMT_BAYER_GBRG8:
      return V4L2_PIX_FMT_SGBRG8;
    case AV_PIX_FMT_BAYER_GRBG8:
      return V4L2_PIX_FMT_SGRBG8;
    case AV_PIX_FMT_BAYER_BGGR16LE:
    case AV_PIX_FMT_BAYER_BGGR16BE:
      return V4L2_PIX_FMT_SBGGR16;
    case AV_PIX_FMT_BAYER_RGGB16LE:
    case AV_PIX_FMT_BAYER_RGGB16BE:
      return V4L2_PIX_FMT_SRGGB16;
    case AV_PIX_FMT_BAYER_GBRG16LE:
    case AV_PIX_FMT_BAYER_GBRG16BE:
      return V4L2_PIX_FMT_SGBRG16;
    case AV_PIX_FMT_BAYER_GRBG16LE:
    case AV_PIX_FMT_BAYER_GRBG16BE:
      return V4L2_PIX_FMT_SGRBG16;
////     case AV_PIX_FMT_XVMC:
//    case AV_PIX_FMT_YUV440P10LE:
//    case AV_PIX_FMT_YUV440P10BE:
//    case AV_PIX_FMT_YUV440P12LE:
//    case AV_PIX_FMT_YUV440P12BE:
//    case AV_PIX_FMT_AYUV64LE:
//    case AV_PIX_FMT_AYUV64BE:
//    case AV_PIX_FMT_VIDEOTOOLBOX:
//    case AV_PIX_FMT_P010LE:
//    case AV_PIX_FMT_P010BE:
//    case AV_PIX_FMT_GBRAP12BE:
//    case AV_PIX_FMT_GBRAP12LE:
//    case AV_PIX_FMT_GBRAP10BE:
//    case AV_PIX_FMT_GBRAP10LE:
//    case AV_PIX_FMT_MEDIACODEC:
//    case AV_PIX_FMT_GRAY12BE:
//    case AV_PIX_FMT_GRAY12LE:
//    case AV_PIX_FMT_GRAY10BE:
//    case AV_PIX_FMT_GRAY10LE:
//    case AV_PIX_FMT_P016LE:
//    case AV_PIX_FMT_P016BE:
//    case AV_PIX_FMT_D3D11:
//    case AV_PIX_FMT_GRAY9BE:
//    case AV_PIX_FMT_GRAY9LE:
//    case AV_PIX_FMT_GBRPF32BE:
//    case AV_PIX_FMT_GBRPF32LE:
//    case AV_PIX_FMT_GBRAPF32BE:
//    case AV_PIX_FMT_GBRAPF32LE:
//    case AV_PIX_FMT_DRM_PRIME:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ffmpeg pixel format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return 0;
}

enum AVPixelFormat
Stream_MediaFramework_Tools::v4lFormatToffmpegFormat (__u32 format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::v4lFormatToffmpegFormat"));

  switch (format_in)
  {
//    case V4L2_PIX_FMT_RGB332:
    case V4L2_PIX_FMT_RGB444:
      return AV_PIX_FMT_RGB444;
//    case V4L2_PIX_FMT_ARGB444:
//    case V4L2_PIX_FMT_XRGB444:
//    case V4L2_PIX_FMT_RGBA444:
//    case V4L2_PIX_FMT_RGBX444:
//    case V4L2_PIX_FMT_ABGR444:
//    case V4L2_PIX_FMT_XBGR444:
//    case V4L2_PIX_FMT_BGRA444:
//    case V4L2_PIX_FMT_BGRX444:
    case V4L2_PIX_FMT_RGB555:
      return AV_PIX_FMT_RGB555;
//    case V4L2_PIX_FMT_ARGB555:
//    case V4L2_PIX_FMT_XRGB555:
//    case V4L2_PIX_FMT_RGBA555:
//    case V4L2_PIX_FMT_RGBX555:
//    case V4L2_PIX_FMT_ABGR555:
//    case V4L2_PIX_FMT_XBGR555:
//    case V4L2_PIX_FMT_BGRA555:
//    case V4L2_PIX_FMT_BGRX555:
    case V4L2_PIX_FMT_RGB565:
      return AV_PIX_FMT_RGB565;
    case V4L2_PIX_FMT_RGB555X:
      return AV_PIX_FMT_RGB555BE;
//    case V4L2_PIX_FMT_ARGB555X:
//    case V4L2_PIX_FMT_XRGB555X:
    case V4L2_PIX_FMT_RGB565X:
      return AV_PIX_FMT_RGB565BE;

    case V4L2_PIX_FMT_BGR666:
      return AV_PIX_FMT_BGR555; // *TODO*: this is wrong
    case V4L2_PIX_FMT_BGR24:
      return AV_PIX_FMT_BGR24;
    case V4L2_PIX_FMT_RGB24:
      return AV_PIX_FMT_RGB24;
    case V4L2_PIX_FMT_BGR32:
      return AV_PIX_FMT_BGRA;
    case V4L2_PIX_FMT_ABGR32:
    case V4L2_PIX_FMT_XBGR32:
      return AV_PIX_FMT_ABGR;
    case V4L2_PIX_FMT_BGRA32:
    case V4L2_PIX_FMT_BGRX32:
      return AV_PIX_FMT_BGRA;
    case V4L2_PIX_FMT_RGB32:
      return AV_PIX_FMT_RGBA;
    case V4L2_PIX_FMT_RGBA32:
    case V4L2_PIX_FMT_RGBX32:
      return AV_PIX_FMT_RGBA;
    case V4L2_PIX_FMT_ARGB32:
    case V4L2_PIX_FMT_XRGB32:
      return AV_PIX_FMT_ARGB;

    case V4L2_PIX_FMT_GREY:
      return AV_PIX_FMT_GRAY8;
//    case V4L2_PIX_FMT_Y4:
//    case V4L2_PIX_FMT_Y6:
//    case V4L2_PIX_FMT_Y10:
//    case V4L2_PIX_FMT_Y12:
//    case V4L2_PIX_FMT_Y14:
    case V4L2_PIX_FMT_Y16:
    case V4L2_PIX_FMT_Y16_BE:
      return AV_PIX_FMT_GRAY16;

//    case V4L2_PIX_FMT_Y10BPACK:
//    case V4L2_PIX_FMT_Y10P:

    case V4L2_PIX_FMT_PAL8:
      return AV_PIX_FMT_PAL8;

//    case V4L2_PIX_FMT_UV8:

    case V4L2_PIX_FMT_YUYV:
      return AV_PIX_FMT_YUYV422;
//    case V4L2_PIX_FMT_YYUV:
    case V4L2_PIX_FMT_YVYU:
      return AV_PIX_FMT_YVYU422;
    case V4L2_PIX_FMT_UYVY:
      return AV_PIX_FMT_UYVY422;
//    case V4L2_PIX_FMT_VYUY:
    case V4L2_PIX_FMT_Y41P:
      return AV_PIX_FMT_YUV411P;
    case V4L2_PIX_FMT_YUV444:
      return AV_PIX_FMT_YUV444P;
//    case V4L2_PIX_FMT_YUV555:
//    case V4L2_PIX_FMT_YUV565:
//    case V4L2_PIX_FMT_YUV24:
//    case V4L2_PIX_FMT_YUV32:
    case V4L2_PIX_FMT_AYUV32:
    case V4L2_PIX_FMT_XYUV32:
      return AV_PIX_FMT_YUVA444P;
//    case V4L2_PIX_FMT_VUYA32:
//    case V4L2_PIX_FMT_VUYX32:
//    case V4L2_PIX_FMT_M420:

    case V4L2_PIX_FMT_NV12:
      return AV_PIX_FMT_NV12;
    case V4L2_PIX_FMT_NV21:
      return AV_PIX_FMT_NV21;
    case V4L2_PIX_FMT_NV16:
      return AV_PIX_FMT_NV16;
//    case V4L2_PIX_FMT_NV61:
//    case V4L2_PIX_FMT_NV24:
//    case V4L2_PIX_FMT_NV42:
//    case V4L2_PIX_FMT_HM12:

//    case V4L2_PIX_FMT_NV12M:
//    case V4L2_PIX_FMT_NV21M:
//    case V4L2_PIX_FMT_NV16M:
//    case V4L2_PIX_FMT_NV61M:
//    case V4L2_PIX_FMT_NV12MT:
//    case V4L2_PIX_FMT_NV12MT_16X16:

    case V4L2_PIX_FMT_YUV410:
      return AV_PIX_FMT_YUV410P;
    case V4L2_PIX_FMT_YVU410:
      return AV_PIX_FMT_YUV410P; // *TODO*: this is wrong
    case V4L2_PIX_FMT_YUV411P:
      return AV_PIX_FMT_YUV411P;
    case V4L2_PIX_FMT_YUV420:
      return AV_PIX_FMT_YUV420P;
    case V4L2_PIX_FMT_YVU420: // 'YV12'
      return AV_PIX_FMT_YUV420P; // *TODO*: this is wrong
    case V4L2_PIX_FMT_YUV422P:
      return AV_PIX_FMT_YUV422P;

//    case V4L2_PIX_FMT_YUV420M:
//    case V4L2_PIX_FMT_YVU420M:
//    case V4L2_PIX_FMT_YUV422M:
//    case V4L2_PIX_FMT_YVU422M:
//    case V4L2_PIX_FMT_YUV444M:
//    case V4L2_PIX_FMT_YVU444M:

    case V4L2_PIX_FMT_SBGGR8:
      return AV_PIX_FMT_BAYER_BGGR8;
    case V4L2_PIX_FMT_SGBRG8:
      return AV_PIX_FMT_BAYER_GBRG8;
    case V4L2_PIX_FMT_SGRBG8:
      return AV_PIX_FMT_BAYER_GRBG8;
    case V4L2_PIX_FMT_SRGGB8:
      return AV_PIX_FMT_BAYER_RGGB8;
//    case V4L2_PIX_FMT_SBGGR10:
//    case V4L2_PIX_FMT_SGBRG10:
//    case V4L2_PIX_FMT_SGRBG10:
//    case V4L2_PIX_FMT_SRGGB10:

//    case V4L2_PIX_FMT_SBGGR10P:
//    case V4L2_PIX_FMT_SGBRG10P:
//    case V4L2_PIX_FMT_SGRBG10P:
//    case V4L2_PIX_FMT_SRGGB10P:

//    case V4L2_PIX_FMT_SBGGR10ALAW8:
//    case V4L2_PIX_FMT_SGBRG10ALAW8:
//    case V4L2_PIX_FMT_SGRBG10ALAW8:
//    case V4L2_PIX_FMT_SRGGB10ALAW8:

//    case V4L2_PIX_FMT_SBGGR10DPCM8:
//    case V4L2_PIX_FMT_SGBRG10DPCM8:
//    case V4L2_PIX_FMT_SGRBG10DPCM8:
//    case V4L2_PIX_FMT_SRGGB10DPCM8:
//    case V4L2_PIX_FMT_SBGGR12:
//    case V4L2_PIX_FMT_SGBRG12:
//    case V4L2_PIX_FMT_SGRBG12:
//    case V4L2_PIX_FMT_SRGGB12:

//    case V4L2_PIX_FMT_SBGGR12P:
//    case V4L2_PIX_FMT_SGBRG12P:
//    case V4L2_PIX_FMT_SGRBG12P:
//    case V4L2_PIX_FMT_SRGGB12P:
//    case V4L2_PIX_FMT_SBGGR14:
//    case V4L2_PIX_FMT_SGBRG14:
//    case V4L2_PIX_FMT_SGRBG14:
//    case V4L2_PIX_FMT_SRGGB14:

//    case V4L2_PIX_FMT_SBGGR14P:
//    case V4L2_PIX_FMT_SGBRG14P:
//    case V4L2_PIX_FMT_SGRBG14P:
//    case V4L2_PIX_FMT_SRGGB14P:
    case V4L2_PIX_FMT_SBGGR16:
      return AV_PIX_FMT_BAYER_BGGR16;
//    case V4L2_PIX_FMT_SGBRG16:
//    case V4L2_PIX_FMT_SGRBG16:
//    case V4L2_PIX_FMT_SRGGB16:

//    case V4L2_PIX_FMT_HSV24:
//    case V4L2_PIX_FMT_HSV32:

    case V4L2_PIX_FMT_MJPEG:
      // *NOTE*: "... MJPEG, or at least the MJPEG in AVIs having the MJPG
      //         fourcc, is restricted JPEG with a fixed -- and *omitted* --
      //         Huffman table. The JPEG must be YCbCr colorspace, it must be
      //         4:2:2, and it must use basic Huffman encoding, not arithmetic
      //         or progressive. . . . You can indeed extract the MJPEG frames
      //         and decode them with a regular JPEG decoder, but you have to
      //         prepend the DHT segment to them, or else the decoder won't
      //         have any idea how to decompress the data. The exact table
      //         necessary is given in the OpenDML spec. ..."
      // *TODO*: libav doesn't specify a pixel format for MJPEG (it is a codec)
      return AV_PIX_FMT_YUVJ422P;
//    case V4L2_PIX_FMT_JPEG:
//    case V4L2_PIX_FMT_DV:
//    case V4L2_PIX_FMT_MPEG:
//    case V4L2_PIX_FMT_H264:
//    case V4L2_PIX_FMT_H264_NO_SC:
//    case V4L2_PIX_FMT_H264_MVC:
//    case V4L2_PIX_FMT_H263:
//    case V4L2_PIX_FMT_MPEG1:
//    case V4L2_PIX_FMT_MPEG2:
//    case V4L2_PIX_FMT_MPEG2_SLICE:
//    case V4L2_PIX_FMT_MPEG4:
//    case V4L2_PIX_FMT_XVID:
//    case V4L2_PIX_FMT_VC1_ANNEX_G:
//    case V4L2_PIX_FMT_VC1_ANNEX_L:
//    case V4L2_PIX_FMT_VP8:
//    case V4L2_PIX_FMT_VP8_FRAME:
//    case V4L2_PIX_FMT_VP9:
//    case V4L2_PIX_FMT_HEVC:
//    case V4L2_PIX_FMT_FWHT:
//    case V4L2_PIX_FMT_FWHT_STATELESS:
//    case V4L2_PIX_FMT_H264_SLICE:

//    case V4L2_PIX_FMT_CPIA1:
//    case V4L2_PIX_FMT_WNVA:
//    case V4L2_PIX_FMT_SN9C10X:
//    case V4L2_PIX_FMT_SN9C20X_I420:
//    case V4L2_PIX_FMT_PWC1:
//    case V4L2_PIX_FMT_PWC2:
//    case V4L2_PIX_FMT_ET61X251:
//    case V4L2_PIX_FMT_SPCA501:
//    case V4L2_PIX_FMT_SPCA505:
//    case V4L2_PIX_FMT_SPCA508:
//    case V4L2_PIX_FMT_SPCA561:
//    case V4L2_PIX_FMT_PAC207:
//    case V4L2_PIX_FMT_MR97310A:
//    case V4L2_PIX_FMT_JL2005BCD:
//    case V4L2_PIX_FMT_SN9C2028:
//    case V4L2_PIX_FMT_SQ905C:
//    case V4L2_PIX_FMT_PJPG:
//    case V4L2_PIX_FMT_OV511:
//    case V4L2_PIX_FMT_OV518:
//    case V4L2_PIX_FMT_STV0680:
//    case V4L2_PIX_FMT_TM6000:
//    case V4L2_PIX_FMT_CIT_YYVYUY:
//    case V4L2_PIX_FMT_KONICA420:
//    case V4L2_PIX_FMT_JPGL:
//    case V4L2_PIX_FMT_SE401:
//    case V4L2_PIX_FMT_S5C_UYVY_JPG:
//    case V4L2_PIX_FMT_Y8I:
//    case V4L2_PIX_FMT_Y12I:
//    case V4L2_PIX_FMT_Z16:
//    case V4L2_PIX_FMT_MT21C:
//    case V4L2_PIX_FMT_INZI:
//    case V4L2_PIX_FMT_SUNXI_TILED_NV12:
//    case V4L2_PIX_FMT_CNF4:
//    case V4L2_PIX_FMT_HI240:

//    case V4L2_PIX_FMT_IPU3_SBGGR10:
//    case V4L2_PIX_FMT_IPU3_SGBRG10:
//    case V4L2_PIX_FMT_IPU3_SGRBG10:
//    case V4L2_PIX_FMT_IPU3_SRGGB10:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown v4l2 pixel format (was: \"%s\" [%d]), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::v4lFormatToString (format_in).c_str ()), format_in));
      break;
    }
  } // end SWITCH

  return AV_PIX_FMT_NONE;
}
#endif // FFMPEG_SUPPORT

unsigned int
Stream_MediaFramework_Tools::frameSize (const std::string& deviceIdentifier_in,
                                        const struct Stream_MediaFramework_V4L_MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::frameSize"));

  int fd = -1;
  int open_mode = O_RDONLY;
  fd = ACE_OS::open (deviceIdentifier_in.c_str (),
                     open_mode);
  if (fd == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::open(\"%s\",%u): \"%m\", aborting\n"),
                ACE_TEXT (deviceIdentifier_in.c_str ()), open_mode));
    return 0;
  } // end IF

  struct v4l2_format format_s;
  ACE_OS::memset (&format_s, 0, sizeof (struct v4l2_format));
  format_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int result = ACE_OS::ioctl (fd,
                              VIDIOC_G_FMT,
                              &format_s);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fd, ACE_TEXT ("VIDIOC_G_FMT")));
    goto error;
  } // end IF
  format_s.fmt.pix.pixelformat = mediaType_in.format.pixelformat;
  format_s.fmt.pix.width = mediaType_in.format.width;
  format_s.fmt.pix.height = mediaType_in.format.height;

  format_s.fmt.pix.bytesperline = 0;
//  format_s.fmt.pix.priv = 0;
  format_s.fmt.pix.sizeimage = 0;
  result = ACE_OS::ioctl (fd,
                       VIDIOC_TRY_FMT,
                       &format_s);
  if (result == -1)
  {// int error = ACE_OS::last_error (); ACE_UNUSED_ARG (error);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,%s): \"%m\", aborting\n"),
                fd, ACE_TEXT ("VIDIOC_TRY_FMT")));
    goto error;
  } // end IF

  result = ACE_OS::close (fd);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                fd));

  return static_cast<unsigned int> (format_s.fmt.pix.sizeimage);

error:
  if (fd != -1)
  {
    result = ACE_OS::close (fd);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  fd));
  } // end IF

  return 0;
}

#if defined (LIBCAMERA_SUPPORT)
#if defined (FFMPEG_SUPPORT)
libcamera::PixelFormat
Stream_MediaFramework_Tools::ffmpegFormatToLibCameraFormat (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::ffmpegFormatToLibCameraFormat"));

  switch (format_in)
  {
    case AV_PIX_FMT_YUV420P:
      return libcamera::formats::YUV420;
    case AV_PIX_FMT_YUYV422:
      return libcamera::formats::YUV422;
    case AV_PIX_FMT_RGB24:
      return libcamera::formats::RGB888;
    case AV_PIX_FMT_BGR24:
      return libcamera::formats::BGR888;
    case AV_PIX_FMT_YUV422P:
      return libcamera::formats::YUV422;
    case AV_PIX_FMT_YUV444P:
      return libcamera::formats::YUV422;
    case AV_PIX_FMT_YUV410P:
      return libcamera::formats::YUV422;
    case AV_PIX_FMT_YUV411P:
      return libcamera::formats::YUV422;
    case AV_PIX_FMT_GRAY8:
      return libcamera::formats::R8;
//    case AV_PIX_FMT_MONOWHITE:
//    case AV_PIX_FMT_MONOBLACK:
    case AV_PIX_FMT_PAL8:
      return libcamera::formats::R8;
//    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVJ422P:
      // *TODO*: libav doesn't specify a pixel format for MJPEG (it is a codec)
      return libcamera::formats::MJPEG;
//    case AV_PIX_FMT_YUVJ444P:
//    case AV_PIX_FMT_XVMC_MPEG2_MC:
//    case AV_PIX_FMT_XVMC_MPEG2_IDCT:
//    case AV_PIX_FMT_XVMC:
    case AV_PIX_FMT_UYVY422:
      return libcamera::formats::UYVY;
//    case AV_PIX_FMT_UYYVYY411:
//    case AV_PIX_FMT_BGR8:
//    case AV_PIX_FMT_BGR4:
//    case AV_PIX_FMT_BGR4_BYTE:
//    case AV_PIX_FMT_RGB8:
//    case AV_PIX_FMT_RGB4:
//    case AV_PIX_FMT_RGB4_BYTE:
    case AV_PIX_FMT_NV12:
      return libcamera::formats::NV12;
    case AV_PIX_FMT_NV21:
      return libcamera::formats::NV21;
    case AV_PIX_FMT_ARGB:
      return libcamera::formats::ARGB8888;
//    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_ABGR:
      return libcamera::formats::ABGR8888;
    case AV_PIX_FMT_BGRA:
      return libcamera::formats::BGRA8888;
//    case AV_PIX_FMT_GRAY16BE:
//    case AV_PIX_FMT_GRAY16LE:
//    case AV_PIX_FMT_YUV440P:
//    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUVA420P:
      return libcamera::formats::YUV420;
//    case AV_PIX_FMT_VDPAU_H264:
//    case AV_PIX_FMT_VDPAU_MPEG1:
//    case AV_PIX_FMT_VDPAU_MPEG2:
//    case AV_PIX_FMT_VDPAU_WMV3:
//    case AV_PIX_FMT_VDPAU_VC1:
//     case AV_PIX_FMT_RGB48BE:
//     case AV_PIX_FMT_RGB48LE:
     case AV_PIX_FMT_RGB565BE:
      return libcamera::formats::RGB565;
    case AV_PIX_FMT_RGB565LE:
      return libcamera::formats::RGB565;
//    case AV_PIX_FMT_RGB555BE:
//    case AV_PIX_FMT_RGB555LE:
//    case AV_PIX_FMT_BGR565BE:
//    case AV_PIX_FMT_BGR565LE:
//    case AV_PIX_FMT_BGR555BE:
//    case AV_PIX_FMT_BGR555LE:
//    case AV_PIX_FMT_VAAPI_MOCO:
//    case AV_PIX_FMT_VAAPI_IDCT:
//    case AV_PIX_FMT_VAAPI_VLD:
//    case AV_PIX_FMT_VAAPI:
//    case AV_PIX_FMT_YUV420P16LE:
//    case AV_PIX_FMT_YUV420P16BE:
//    case AV_PIX_FMT_YUV422P16LE:
//    case AV_PIX_FMT_YUV422P16BE:
//    case AV_PIX_FMT_YUV444P16LE:
//    case AV_PIX_FMT_YUV444P16BE:
//    case AV_PIX_FMT_VDPAU_MPEG4:
//    case AV_PIX_FMT_DXVA2_VLD:
//    case AV_PIX_FMT_RGB444LE:
//    case AV_PIX_FMT_RGB444BE:
//    case AV_PIX_FMT_BGR444LE:
//    case AV_PIX_FMT_BGR444BE:
//    case AV_PIX_FMT_YA8:
//    case AV_PIX_FMT_Y400A:
//    case AV_PIX_FMT_GRAY8A:
//    case AV_PIX_FMT_BGR48BE:
//    case AV_PIX_FMT_BGR48LE:
//    case AV_PIX_FMT_YUV420P9BE:
//    case AV_PIX_FMT_YUV420P9LE:
//    case AV_PIX_FMT_YUV420P10BE:
//    case AV_PIX_FMT_YUV420P10LE:
//    case AV_PIX_FMT_YUV422P10BE:
//    case AV_PIX_FMT_YUV422P10LE:
//    case AV_PIX_FMT_YUV444P9BE:
//    case AV_PIX_FMT_YUV444P9LE:
//    case AV_PIX_FMT_YUV444P10BE:
//    case AV_PIX_FMT_YUV444P10LE:
//    case AV_PIX_FMT_YUV422P9BE:
//    case AV_PIX_FMT_YUV422P9LE:
//    case AV_PIX_FMT_VDA_VLD:
//    case AV_PIX_FMT_GBRP:
//    case AV_PIX_FMT_GBR24P:
//    case AV_PIX_FMT_GBRP9BE:
//    case AV_PIX_FMT_GBRP9LE:
//    case AV_PIX_FMT_GBRP10BE:
//    case AV_PIX_FMT_GBRP10LE:
//    case AV_PIX_FMT_GBRP16BE:
//    case AV_PIX_FMT_GBRP16LE:
//    case AV_PIX_FMT_YUVA422P:
//    case AV_PIX_FMT_YUVA444P:
//    case AV_PIX_FMT_YUVA420P9BE:
//    case AV_PIX_FMT_YUVA420P9LE:
//    case AV_PIX_FMT_YUVA422P9BE:
//    case AV_PIX_FMT_YUVA422P9LE:
//    case AV_PIX_FMT_YUVA444P9BE:
//    case AV_PIX_FMT_YUVA444P9LE:
//    case AV_PIX_FMT_YUVA420P10BE:
//    case AV_PIX_FMT_YUVA420P10LE:
//    case AV_PIX_FMT_YUVA422P10BE:
//    case AV_PIX_FMT_YUVA422P10LE:
//    case AV_PIX_FMT_YUVA444P10BE:
//    case AV_PIX_FMT_YUVA444P10LE:
//    case AV_PIX_FMT_YUVA420P16BE:
//    case AV_PIX_FMT_YUVA420P16LE:
//    case AV_PIX_FMT_YUVA422P16BE:
//    case AV_PIX_FMT_YUVA422P16LE:
//    case AV_PIX_FMT_YUVA444P16BE:
//    case AV_PIX_FMT_YUVA444P16LE:
//    case AV_PIX_FMT_VDPAU:
//    case AV_PIX_FMT_XYZ12LE:
//    case AV_PIX_FMT_XYZ12BE:
    case AV_PIX_FMT_NV16:
      return libcamera::formats::NV16;
//    case AV_PIX_FMT_NV20LE:
//    case AV_PIX_FMT_NV20BE:
//    case AV_PIX_FMT_RGBA64BE:
//    case AV_PIX_FMT_RGBA64LE:
//    case AV_PIX_FMT_BGRA64BE:
//    case AV_PIX_FMT_BGRA64LE:
    case AV_PIX_FMT_YVYU422:
      return libcamera::formats::YVYU;
//    case AV_PIX_FMT_VDA:
//    case AV_PIX_FMT_YA16BE:
//    case AV_PIX_FMT_YA16LE:
//    case AV_PIX_FMT_GBRAP:
//    case AV_PIX_FMT_GBRAP16BE:
//    case AV_PIX_FMT_GBRAP16LE:
//    case AV_PIX_FMT_QSV:
//    case AV_PIX_FMT_MMAL:
//    case AV_PIX_FMT_D3D11VA_VLD:
//    case AV_PIX_FMT_CUDA:
//    case AV_PIX_FMT_0RGB:
//    case AV_PIX_FMT_RGB0:
//    case AV_PIX_FMT_0BGR:
//    case AV_PIX_FMT_BGR0:
//    case AV_PIX_FMT_YUV420P12BE:
//    case AV_PIX_FMT_YUV420P12LE:
//    case AV_PIX_FMT_YUV420P14BE:
//    case AV_PIX_FMT_YUV420P14LE:
//    case AV_PIX_FMT_YUV422P12BE:
//    case AV_PIX_FMT_YUV422P12LE:
//    case AV_PIX_FMT_YUV422P14BE:
//    case AV_PIX_FMT_YUV422P14LE:
//    case AV_PIX_FMT_YUV444P12BE:
//    case AV_PIX_FMT_YUV444P12LE:
//    case AV_PIX_FMT_YUV444P14BE:
//    case AV_PIX_FMT_YUV444P14LE:
//    case AV_PIX_FMT_GBRP12BE:
//    case AV_PIX_FMT_GBRP12LE:
//    case AV_PIX_FMT_GBRP14BE:
//    case AV_PIX_FMT_GBRP14LE:
//    case AV_PIX_FMT_YUVJ411P:
    case AV_PIX_FMT_BAYER_BGGR8:
      return libcamera::formats::SBGGR8;
    case AV_PIX_FMT_BAYER_RGGB8:
      return libcamera::formats::SRGGB8;
    case AV_PIX_FMT_BAYER_GBRG8:
      return libcamera::formats::SGBRG8;
    case AV_PIX_FMT_BAYER_GRBG8:
      return libcamera::formats::SGRBG8;
    case AV_PIX_FMT_BAYER_BGGR16LE:
    case AV_PIX_FMT_BAYER_BGGR16BE:
      return libcamera::formats::SBGGR16;
    case AV_PIX_FMT_BAYER_RGGB16LE:
    case AV_PIX_FMT_BAYER_RGGB16BE:
      return libcamera::formats::SRGGB16;
    case AV_PIX_FMT_BAYER_GBRG16LE:
    case AV_PIX_FMT_BAYER_GBRG16BE:
      return libcamera::formats::SGBRG16;
    case AV_PIX_FMT_BAYER_GRBG16LE:
    case AV_PIX_FMT_BAYER_GRBG16BE:
      return libcamera::formats::SGRBG16;
////     case AV_PIX_FMT_XVMC:
//    case AV_PIX_FMT_YUV440P10LE:
//    case AV_PIX_FMT_YUV440P10BE:
//    case AV_PIX_FMT_YUV440P12LE:
//    case AV_PIX_FMT_YUV440P12BE:
//    case AV_PIX_FMT_AYUV64LE:
//    case AV_PIX_FMT_AYUV64BE:
//    case AV_PIX_FMT_VIDEOTOOLBOX:
//    case AV_PIX_FMT_P010LE:
//    case AV_PIX_FMT_P010BE:
//    case AV_PIX_FMT_GBRAP12BE:
//    case AV_PIX_FMT_GBRAP12LE:
//    case AV_PIX_FMT_GBRAP10BE:
//    case AV_PIX_FMT_GBRAP10LE:
//    case AV_PIX_FMT_MEDIACODEC:
//    case AV_PIX_FMT_GRAY12BE:
//    case AV_PIX_FMT_GRAY12LE:
//    case AV_PIX_FMT_GRAY10BE:
//    case AV_PIX_FMT_GRAY10LE:
//    case AV_PIX_FMT_P016LE:
//    case AV_PIX_FMT_P016BE:
//    case AV_PIX_FMT_D3D11:
//    case AV_PIX_FMT_GRAY9BE:
//    case AV_PIX_FMT_GRAY9LE:
//    case AV_PIX_FMT_GBRPF32BE:
//    case AV_PIX_FMT_GBRPF32LE:
//    case AV_PIX_FMT_GBRAPF32BE:
//    case AV_PIX_FMT_GBRAPF32LE:
//    case AV_PIX_FMT_DRM_PRIME:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ffmpeg pixel format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return libcamera::PixelFormat (0, 0);
}

enum AVPixelFormat
Stream_MediaFramework_Tools::libCameraFormatToffmpegFormat (const libcamera::PixelFormat& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_Tools::libCameraFormatToffmpegFormat"));

  switch (format_in)
  {
    case libcamera::formats::R8:
      return AV_PIX_FMT_GRAY8;
    case libcamera::formats::RGB565:
      return AV_PIX_FMT_RGB565;
    case libcamera::formats::RGB888:
      return AV_PIX_FMT_RGB24;
    case libcamera::formats::BGR888:
      return AV_PIX_FMT_BGR24;
    case libcamera::formats::XRGB8888:
      return AV_PIX_FMT_ARGB;
    case libcamera::formats::XBGR8888:
      return AV_PIX_FMT_ABGR;
    case libcamera::formats::RGBX8888:
      return AV_PIX_FMT_RGBA;
    case libcamera::formats::BGRX8888:
      return AV_PIX_FMT_BGRA;
    case libcamera::formats::ARGB8888:
      return AV_PIX_FMT_ARGB;
    case libcamera::formats::ABGR8888:
      return AV_PIX_FMT_ABGR;
    case libcamera::formats::RGBA8888:
      return AV_PIX_FMT_RGBA;
    case libcamera::formats::BGRA8888:
      return AV_PIX_FMT_BGRA;
    case libcamera::formats::YUYV:
      return AV_PIX_FMT_YUYV422;
    case libcamera::formats::YVYU:
      return AV_PIX_FMT_YVYU422;
    case libcamera::formats::UYVY:
      return AV_PIX_FMT_UYVY422;
    case libcamera::formats::VYUY:
      return AV_PIX_FMT_UYVY422;
    case libcamera::formats::NV12:
      return AV_PIX_FMT_NV12;
    case libcamera::formats::NV21:
      return AV_PIX_FMT_NV21;
    case libcamera::formats::NV16:
      return AV_PIX_FMT_NV16;
    case libcamera::formats::NV61:
      return AV_PIX_FMT_NV16;
    case libcamera::formats::NV24:
      return AV_PIX_FMT_NV24;
    case libcamera::formats::NV42:
      return AV_PIX_FMT_NV42;
    case libcamera::formats::YUV420:
      return AV_PIX_FMT_YUV420P;
    case libcamera::formats::YVU420:
      return AV_PIX_FMT_YUV420P; // *TODO*: this is wrong
    case libcamera::formats::YUV422:
      return AV_PIX_FMT_YUV422P;
    case libcamera::formats::MJPEG:
      return AV_PIX_FMT_YUVJ422P; // *TODO*: this is wrong
    case libcamera::formats::SRGGB8:
      return AV_PIX_FMT_BAYER_RGGB8;
    case libcamera::formats::SGRBG8:
      return AV_PIX_FMT_BAYER_GRBG8;
    case libcamera::formats::SGBRG8:
      return AV_PIX_FMT_BAYER_GBRG8;
    case libcamera::formats::SBGGR8:
      return AV_PIX_FMT_BAYER_BGGR8;
//    case libcamera::formats::SRGGB10:
//    case libcamera::formats::SGRBG10:
//    case libcamera::formats::SGBRG10:
//    case libcamera::formats::SBGGR10:
//    case libcamera::formats::SRGGB12:
//    case libcamera::formats::SGRBG12:
//    case libcamera::formats::SGBRG12:
//    case libcamera::formats::SBGGR12:
    case libcamera::formats::SRGGB16:
      return AV_PIX_FMT_BAYER_RGGB16;
    case libcamera::formats::SGRBG16:
      return AV_PIX_FMT_BAYER_GRBG16;
    case libcamera::formats::SGBRG16:
      return AV_PIX_FMT_BAYER_GBRG16;
    case libcamera::formats::SBGGR16:
      return AV_PIX_FMT_BAYER_BGGR16;
//    case libcamera::formats::SRGGB10_CSI2P:
//    case libcamera::formats::SGRBG10_CSI2P:
//    case libcamera::formats::SGBRG10_CSI2P:
//    case libcamera::formats::SBGGR10_CSI2P:
//    case libcamera::formats::SRGGB12_CSI2P:
//    case libcamera::formats::SGRBG12_CSI2P:
//    case libcamera::formats::SGBRG12_CSI2P:
//    case libcamera::formats::SBGGR12_CSI2P:
//    case libcamera::formats::SRGGB10_IPU3:
//    case libcamera::formats::SGRBG10_IPU3:
//    case libcamera::formats::SGBRG10_IPU3:
//    case libcamera::formats::SBGGR10_IPU3:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown libcamera pixel format (was: \"%s\"), aborting\n"),
                  ACE_TEXT (format_in.toString().c_str ())));
      break;
    }
  } // end SWITCH

  return AV_PIX_FMT_NONE;
}
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
