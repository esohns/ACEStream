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
#include "stream_vis_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <dshow.h>
#include <dvdmedia.h>
//#include <evr.h>
//#include <ksuuids.h>
#include <mfapi.h>
//#include <mtype.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
//#include <wmcodecdsp.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_lib_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_vis_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_Visualization_Tools::initialize (enum Stream_Visualization_Framework framework_in,
                                        bool initializeCOM_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::initialize"));

  bool result = false;

  switch (framework_in)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTDRAW:
    {
      result =
        Stream_MediaFramework_DirectDraw_Tools::initialize (initializeCOM_in);
      if (!result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectDraw_Tools::initialize(), aborting\n")));
        return false;
      }
      break;
    }
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_GDI:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_MEDIAFOUNDATION:
      break;
#endif // ACE_WIN32 || ACE_WIN64
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown visualization framework (was: %d), aborting\n"),
                  framework_in));
      return false;
    }
  } // end SWITCH

  return result;
}
void
Stream_Visualization_Tools::finalize (enum Stream_Visualization_Framework framework_in,
                                      bool initializeCOM_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::finalize"));

  switch (framework_in)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTDRAW:
    {
      Stream_MediaFramework_DirectDraw_Tools::finalize (initializeCOM_in);
      break;
    }
    case STREAM_VISUALIZATION_FRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_GDI:
      break;
    case STREAM_VISUALIZATION_FRAMEWORK_MEDIAFOUNDATION:
      break;
#endif // ACE_WIN32 || ACE_WIN64
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown visualization framework (was: %d), returning\n"),
                  framework_in));
      return;
    }
  } // end SWITCH
}
#endif // ACE_WIN32 || ACE_WIN64

std::string
Stream_Visualization_Tools::rendererToModuleName (enum Stream_Visualization_AudioRenderer renderer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::rendererToModuleName"));

  std::string result;

  switch (renderer_in)
  {
    case STREAM_VISUALIZATION_AUDIORENDERER_NULL:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_NULL_DEFAULT_NAME_STRING); break;
#if defined (GTK_SUPPORT)
    case STREAM_VISUALIZATION_AUDIORENDERER_GTK_CAIRO_SPECTRUMANALYZER:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING); break;
#endif // GTK_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown audio renderer (was: %d), aborting\n"),
                  renderer_in));
      break;
    }
  } // end SWITCH

  return result;
}
std::string
Stream_Visualization_Tools::rendererToModuleName (enum Stream_Visualization_VideoRenderer renderer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::rendererToModuleName"));

  std::string result;

  switch (renderer_in)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_NULL:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_NULL_DEFAULT_NAME_STRING); break;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT2D_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GDI_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_MEDIAFOUNDATION:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING); break;
#else
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING); break;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_PIXBUF_DEFAULT_NAME_STRING); break;
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW:
      result = ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_WINDOW_DEFAULT_NAME_STRING); break;
#endif // GTK_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown video renderer (was: %d), aborting\n"),
                  renderer_in));
      break;
    }
  } // end SWITCH

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
enum AVPixelFormat
Stream_Visualization_Tools::mediaSubTypeToAVPixelFormat (REFGUID mediaSubType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_Tools::mediaSubTypeToAVPixelFormat"));

  // DirectShow
  /////////////////////////////////////// AUDIO
  // uncompressed audio
  if (mediaSubType_in == MEDIASUBTYPE_IEEE_FLOAT);
  else if (mediaSubType_in == MEDIASUBTYPE_PCM);
  // MPEG-4 and AAC
  //MEDIASUBTYPE_MPEG_ADTS_AAC
  //MEDIASUBTYPE_MPEG_HEAAC
  //MEDIASUBTYPE_MPEG_LOAS
  //MEDIASUBTYPE_RAW_AAC1

  // Dolby
  //MEDIASUBTYPE_DOLBY_DDPLUS
  //MEDIASUBTYPE_DOLBY_AC3
  //MEDIASUBTYPE_DOLBY_AC3_SPDIF
  //MEDIASUBTYPE_DVM
  //MEDIASUBTYPE_RAW_SPORT
  //MEDIASUBTYPE_SPDIF_TAG_241h

  // miscellaneous
  //MEDIASUBTYPE_DRM_Audio
  //MEDIASUBTYPE_DTS
  //MEDIASUBTYPE_DTS2
  //MEDIASUBTYPE_DVD_LPCM_AUDIO
  //MEDIASUBTYPE_MPEG1AudioPayload
  //MEDIASUBTYPE_MPEG1Packet
  //MEDIASUBTYPE_MPEG1Payload
  //MEDIASUBTYPE_MPEG2_AUDIO
  //MEDIASUBTYPE_PCMAudio_Obsolete
  //MEDIASUBTYPE_MPEG_RAW_AAC

  /////////////////////////////////////// BDA
  //MEDIASUBTYPE_None

  /////////////////////////////////////// DVD
  //MEDIASUBTYPE_DTS
  //MEDIASUBTYPE_DVD_SUBPICTURE
  //MEDIASUBTYPE_SDDS
  //MEDIASUBTYPE_DVD_NAVIGATION_DSI
  //MEDIASUBTYPE_DVD_NAVIGATION_PCI
  //MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER

  /////////////////////////////////////// Line 21
  //MEDIASUBTYPE_Line21_BytePair
  //MEDIASUBTYPE_Line21_GOPPacket
  //MEDIASUBTYPE_Line21_VBIRawData

  /////////////////////////////////////// MPEG-1
  //MEDIASUBTYPE_MPEG1System
  //MEDIASUBTYPE_MPEG1VideoCD
  //MEDIASUBTYPE_MPEG1Packet
  //MEDIASUBTYPE_MPEG1Payload
  //MEDIASUBTYPE_MPEG1Video
  //MEDIASUBTYPE_MPEG1Audio
  //MEDIASUBTYPE_MPEG1AudioPayload

  /////////////////////////////////////// MPEG-2
  // MPEG-2 (splitter)
  //MEDIASUBTYPE_MPEG2_VIDEO
  //MEDIASUBTYPE_DOLBY_AC3
  //MEDIASUBTYPE_DOLBY_AC3_SPDIF
  //MEDIASUBTYPE_MPEG2_AUDIO
  //MEDIASUBTYPE_DVD_LPCM_AUDIO
  // MPEG-2 (demultiplexer)
  //MEDIASUBTYPE_MPEG2_PROGRAM
  //MEDIASUBTYPE_MPEG2_TRANSPORT
  //MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE
  //MEDIASUBTYPE_ATSC_SI
  //MEDIASUBTYPE_DVB_SI
  //MEDIASUBTYPE_ISDB_SI
  //MEDIASUBTYPE_MPEG2DATA
  // MPEG-2 (kernel)

  /////////////////////////////////////// Stream
  //MEDIASUBTYPE_AIFF
  //MEDIASUBTYPE_Asf
  //MEDIASUBTYPE_Avi
  //MEDIASUBTYPE_AU
  //MEDIASUBTYPE_DssAudio
  //MEDIASUBTYPE_DssVideo
  //MEDIASUBTYPE_MPEG1Audio
  //MEDIASUBTYPE_MPEG1System
  //MEDIASUBTYPE_MPEG1SystemStream
  //MEDIASUBTYPE_MPEG1Video
  //MEDIASUBTYPE_MPEG1VideoCD
  //MEDIASUBTYPE_WAVE

  /////////////////////////////////////// VBI
  //KSDATAFORMAT_SUBTYPE_RAW8
  //MEDIASUBTYPE_TELETEXT
  //MEDIASUBTYPE_VPS
  //MEDIASUBTYPE_WSS

  /////////////////////////////////////// VIDEO
  // analog video
  //MEDIASUBTYPE_AnalogVideo_NTSC_M
  //MEDIASUBTYPE_AnalogVideo_PAL_B
  //MEDIASUBTYPE_AnalogVideo_PAL_D
  //MEDIASUBTYPE_AnalogVideo_PAL_G
  //MEDIASUBTYPE_AnalogVideo_PAL_H
  //MEDIASUBTYPE_AnalogVideo_PAL_I
  //MEDIASUBTYPE_AnalogVideo_PAL_M
  //MEDIASUBTYPE_AnalogVideo_PAL_N
  //MEDIASUBTYPE_AnalogVideo_SECAM_B
  //MEDIASUBTYPE_AnalogVideo_SECAM_D
  //MEDIASUBTYPE_AnalogVideo_SECAM_G
  //MEDIASUBTYPE_AnalogVideo_SECAM_H
  //MEDIASUBTYPE_AnalogVideo_SECAM_K
  //MEDIASUBTYPE_AnalogVideo_SECAM_K1
  //MEDIASUBTYPE_AnalogVideo_SECAM_L

  // directx video acceleration
  //MEDIASUBTYPE_AI44
  //MEDIASUBTYPE_IA44

  // DV video
  //MEDIASUBTYPE_dvsl
  //MEDIASUBTYPE_dvsd
  //MEDIASUBTYPE_dvhd

  // H.264
  //MEDIASUBTYPE_AVC1
  //MEDIASUBTYPE_H264
  //MEDIASUBTYPE_h264
  //MEDIASUBTYPE_X264
  //MEDIASUBTYPE_x264

  // uncompressed RGB (no alpha)
  //MEDIASUBTYPE_RGB1
  //MEDIASUBTYPE_RGB4
  //MEDIASUBTYPE_RGB8
  //MEDIASUBTYPE_RGB555
  //MEDIASUBTYPE_RGB565
  //MEDIASUBTYPE_RGB24
  //MEDIASUBTYPE_RGB32
  // uncompressed RGB (alpha)
  //MEDIASUBTYPE_ARGB1555
  //MEDIASUBTYPE_ARGB32
  //MEDIASUBTYPE_ARGB4444
  //MEDIASUBTYPE_A2R10G10B10
  //MEDIASUBTYPE_A2B10G10R10

  // video mixing renderer (VMR-7)
  //MEDIASUBTYPE_RGB32_D3D_DX7_RT
  //MEDIASUBTYPE_RGB16_D3D_DX7_RT
  //MEDIASUBTYPE_ARGB32_D3D_DX7_RT
  //MEDIASUBTYPE_ARGB4444_D3D_DX7_RT
  //MEDIASUBTYPE_ARGB1555_D3D_DX7_RT
  // video mixing renderer (VMR-9)
  //MEDIASUBTYPE_RGB32_D3D_DX9_RT
  //MEDIASUBTYPE_RGB16_D3D_DX9_RT
  //MEDIASUBTYPE_ARGB32_D3D_DX9_RT
  //MEDIASUBTYPE_ARGB4444_D3D_DX9_RT
  //MEDIASUBTYPE_ARGB1555_D3D_DX9_RT

  // YUV video
  //MEDIASUBTYPE_AYUV
  //MEDIASUBTYPE_YUY2
  //MEDIASUBTYPE_UYVY
  //MEDIASUBTYPE_IMC1
  //MEDIASUBTYPE_IMC2
  //MEDIASUBTYPE_IMC3
  //MEDIASUBTYPE_IMC4
  //MEDIASUBTYPE_YV12
  //MEDIASUBTYPE_NV12
  // other YUV
  //MEDIASUBTYPE_I420
  //MEDIASUBTYPE_IF09
  //MEDIASUBTYPE_IYUV
  //MEDIASUBTYPE_Y211
  //MEDIASUBTYPE_Y411
  //MEDIASUBTYPE_Y41P
  //MEDIASUBTYPE_YVU9
  //MEDIASUBTYPE_YVYU
  //MEDIASUBTYPE_YUYV

  // miscellaneous
  //MEDIASUBTYPE_CFCC
  //MEDIASUBTYPE_CLJR
  //MEDIASUBTYPE_CPLA
  //MEDIASUBTYPE_CLPL
  //MEDIASUBTYPE_IJPG
  //MEDIASUBTYPE_MDVF
  //MEDIASUBTYPE_MJPG
  //MEDIASUBTYPE_Overlay
  //MEDIASUBTYPE_Plum
  //MEDIASUBTYPE_QTJpeg
  //MEDIASUBTYPE_QTMovie
  //MEDIASUBTYPE_QTRle
  //MEDIASUBTYPE_QTRpza
  //MEDIASUBTYPE_QTSmc
  //MEDIASUBTYPE_TVMJ
  //MEDIASUBTYPE_VPVBI
  //MEDIASUBTYPE_VPVideo
  //MEDIASUBTYPE_WAKE

  ///////////////////////////////////////
  // unknown
  //MEDIASUBTYPE_DVCS
  //MEDIASUBTYPE_DVSD

  // Media Foundation
  //MFVideoFormat_RGB32
  //MFVideoFormat_ARGB32
  else if (mediaSubType_in == MFVideoFormat_RGB24)
    return AV_PIX_FMT_RGB24;
    //return AV_PIX_FMT_RGB24;
  //MFVideoFormat_RGB555
  //MFVideoFormat_RGB565
  //MFVideoFormat_RGB8
  //MFVideoFormat_AI44
  //MFVideoFormat_AYUV
  else if (mediaSubType_in == MFVideoFormat_YUY2)
    return AV_PIX_FMT_YUYV422;
  //MFVideoFormat_YVYU
  //MFVideoFormat_YVU9
  //MFVideoFormat_UYVY
  //MFVideoFormat_NV11
  //MFVideoFormat_NV12
  // *TODO*: this is wrong...
  else if (mediaSubType_in == MFVideoFormat_YV12)
    return AV_PIX_FMT_NV21;
  //MFVideoFormat_I420
  // *TODO*: endianness of the bytestream may not be that of the host
  else if (mediaSubType_in == MFVideoFormat_IYUV)
    return ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? AV_PIX_FMT_YUV420P16LE
                                                  : AV_PIX_FMT_YUV420P16BE);
  //MFVideoFormat_Y210
  //MFVideoFormat_Y216
  //MFVideoFormat_Y410
  //MFVideoFormat_Y416
  //MFVideoFormat_Y41P
  //MFVideoFormat_Y41T
  //MFVideoFormat_Y42T
  //MFVideoFormat_P210
  //MFVideoFormat_P216
  //MFVideoFormat_P010
  //MFVideoFormat_P016
  //MFVideoFormat_v210
  //MFVideoFormat_v216
  //MFVideoFormat_v410
  //MFVideoFormat_MP43
  //MFVideoFormat_MP4S
  //MFVideoFormat_M4S2
  //MFVideoFormat_MP4V
  //MFVideoFormat_WMV1
  //MFVideoFormat_WMV2
  //MFVideoFormat_WMV3
  //MFVideoFormat_WVC1
  //MFVideoFormat_MSS1
  //MFVideoFormat_MSS2
  //MFVideoFormat_MPG1
  //MFVideoFormat_DVSL
  //MFVideoFormat_DVSD
  //MFVideoFormat_DVHD
  //MFVideoFormat_DV25
  //MFVideoFormat_DV50
  //MFVideoFormat_DVH1
  //MFVideoFormat_DVC
  //MFVideoFormat_H264
  else if (mediaSubType_in == MFVideoFormat_MJPG)
    return AV_PIX_FMT_YUVJ422P;
  //MFVideoFormat_420O
  //MFVideoFormat_HEVC
  //MFVideoFormat_HEVC_ES
#if (WINVER >= _WIN32_WINNT_WIN8)
  //MFVideoFormat_H263
#endif
  //MFVideoFormat_H264_ES
  //MFVideoFormat_MPEG2

  //MFAudioFormat_PCM
  //MFAudioFormat_Float
  //MFAudioFormat_DTS
  //MFAudioFormat_Dolby_AC3_SPDIF
  //MFAudioFormat_DRM
  //MFAudioFormat_WMAudioV8
  //MFAudioFormat_WMAudioV9
  //MFAudioFormat_WMAudio_Lossless
  //MFAudioFormat_WMASPDIF
  //MFAudioFormat_MSP1
  //MFAudioFormat_MP3
  //MFAudioFormat_MPEG
  //MFAudioFormat_AAC
  //MFAudioFormat_ADTS
  //MFAudioFormat_AMR_NB
  //MFAudioFormat_AMR_WB
  //MFAudioFormat_AMR_WP
  //MFAudioFormat_Dolby_AC3
  //MFAudioFormat_Dolby_DDPlus

  return AV_PIX_FMT_NONE;
}
#endif // ACE_WIN32 || ACE_WIN64
