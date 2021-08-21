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

#include "stream_lib_directshow_tools.h"

#include <sstream>

#include <amvideo.h>
// *WARNING*: "...Note Header files ksproxy.h and dsound.h define similar but
//            incompatible versions of the IKsPropertySet interface.Applications
//            that require the KS proxy module should use the version defined in
//            ksproxy.h.The DirectSound version of IKsPropertySet is described
//            in the DirectSound reference pages in the Microsoft Windows SDK
//            documentation.
//            If an application must include both ksproxy.h and dsound.h,
//            whichever header file the compiler scans first is the one whose
//            definition of IKsPropertySet is used by the compiler. ..."
//#include <dsound.h>
//#include <dxva.h>
#include <fourcc.h>
#include <Ks.h>
#include <ksmedia.h>
#include <KsProxy.h>
#include <dmoreg.h>
#include <Dmodshow.h>
#include <dvdmedia.h>
//#include <ksuuids.h>
#include <mmreg.h>
#include <oleauto.h>
#include <qedit.h>
#include <strsafe.h>
//#include <strmif.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#include <vfwmsgs.h>
#include <wmcodecdsp.h>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common.h"
#include "common_time_common.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

//#include "stream_dec_defines.h"
//#include "stream_dec_tools.h"

//#include "stream_dev_defines.h"

#include "stream_lib_tools.h"

// initialize statics
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap;
Stream_MediaFramework_DirectShow_Tools::WORD_TO_STRING_MAP_T Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap;
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap;
ACE_HANDLE Stream_MediaFramework_DirectShow_Tools::logFileHandle = ACE_INVALID_HANDLE;

bool
Stream_MediaFramework_DirectShow_Tools::initialize (bool coInitialize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::initialize"));

  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_Video, ACE_TEXT_ALWAYS_CHAR ("vids")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_Audio, ACE_TEXT_ALWAYS_CHAR ("auds")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_Text, ACE_TEXT_ALWAYS_CHAR ("txts")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_Midi, ACE_TEXT_ALWAYS_CHAR ("mids")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_Stream, ACE_TEXT_ALWAYS_CHAR ("Stream")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_Interleaved, ACE_TEXT_ALWAYS_CHAR ("iavs")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_File, ACE_TEXT_ALWAYS_CHAR ("file")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_ScriptCommand, ACE_TEXT_ALWAYS_CHAR ("scmd")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_AUXLine21Data, ACE_TEXT_ALWAYS_CHAR ("AUXLine21Data")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_AUXTeletextPage, ACE_TEXT_ALWAYS_CHAR ("AUXTeletextPage")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_CC_CONTAINER, ACE_TEXT_ALWAYS_CHAR ("CC_CONTAINER")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_DTVCCData, ACE_TEXT_ALWAYS_CHAR ("DTVCCData")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_MSTVCaption, ACE_TEXT_ALWAYS_CHAR ("MSTVCaption")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_VBI, ACE_TEXT_ALWAYS_CHAR ("VBI")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_Timecode, ACE_TEXT_ALWAYS_CHAR ("Timecode")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_LMRT, ACE_TEXT_ALWAYS_CHAR ("lmrt")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_URL_STREAM, ACE_TEXT_ALWAYS_CHAR ("URL_STREAM")));

  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PACK, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PACK")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PES, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PES")));
#if ( (NTDDI_VERSION >= NTDDI_WINXPSP2) && (NTDDI_VERSION < NTDDI_WS03) ) || (NTDDI_VERSION >= NTDDI_WS03SP1)
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_MPEG2_SECTIONS, ACE_TEXT_ALWAYS_CHAR ("MPEG2_SECTIONS")));
#endif
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PACK, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PACK")));

  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_DVD_ENCRYPTED_PACK, ACE_TEXT_ALWAYS_CHAR ("DVD_ENCRYPTED_PACK")));
  Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MEDIATYPE_DVD_NAVIGATION, ACE_TEXT_ALWAYS_CHAR ("DVD_NAVIGATION")));

  // ---------------------------------------------------------------------------

  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNKNOWN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VSELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IBM_CVSD, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ALAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MULAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DTS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DRM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAVOICE9, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAVOICE10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OKI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DVI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IMA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MEDIASPACE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIERRA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G723_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGISTD, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGIFIX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIALOGIC_OKI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MEDIAVISION_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CU_CODEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_HP_DYN_VOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_YAMAHA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONARC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DSPGROUP_TRUESPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ECHOSC1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AUDIOFILE_AF36, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_APTX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AUDIOFILE_AF10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PROSODY_1612, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LRC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DOLBY_AC2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSNAUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ANTEX_ADPCME, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONTROL_RES_VQLPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGIREAL, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGIADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONTROL_RES_CR10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NMS_VBXADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CS_IMAADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ECHOSC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ROCKWELL_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ROCKWELL_DIGITALK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_XEBEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G721_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G728_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSG723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SHARP_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEGLAYER3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CIRRUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ESPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CANOPUS_ATRAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G726_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G722_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DSAT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DSAT_DISPLAY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_BYTE_ALIGNED, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC16, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC20, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT29, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT29HW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_VR12, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_VR18, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_TQ40, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_SC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_SC3_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SOFTSOUND, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_TQ60, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSRT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G729A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MVI_MVI2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DF_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DF_GSM610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ISIAUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ONLIVE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MULTITUDE_FT_SX20, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INFOCOM_ITS_G721_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONVEDIA_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONGRUENCY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SBC24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MEDIASONIC_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PROSODY_8KBPS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ZYXEL_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_LPCBB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PACKED, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MALDEN_PHONYTALK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_GSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_G720_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_TETRA_ACELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NEC_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RAW_AAC1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RHETOREX_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IRAT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VIVO_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VIVO_SIREN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_GRUNDIG, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGITAL_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SANYO_LD_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACEPLNET, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACELP4800, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACELP8V3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_G729A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_KELVIN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOICEAGE_AMR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G726ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DICTAPHONE_CELP68, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DICTAPHONE_CELP54, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QUALCOMM_PUREVOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QUALCOMM_HALFRATE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TUBGSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSAUDIO1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMASPDIF, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ULAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ALAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_16K, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC008, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_G726L, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_CELP54, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_CELP68, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_KNOWLEDGE_ADVENTURE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FRAUNHOFER_IIS_MPEG2_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DTS_DS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_FASTSPEECH8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_FASTSPEECH10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UHER_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ULEAD_DV_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ULEAD_DV_AUDIO_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QUARTERDECK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ILINK_VC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RAW_SPORT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ESST_AC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GENERIC_PASSTHRU, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IPI_HSX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IPI_RPELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CS2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SCX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SCY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_ATRAC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TELUM_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TELUM_IA_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NORCOM_VOICE_SYSTEMS_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FM_TOWNS_SND, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MICRONAS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MICRONAS_CELP833, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_BTV_DIGITAL, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_MUSIC_CODER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INDEO_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QDESIGN_MUSIC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ON2_VP7_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ON2_VP6_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VME_VMPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LIGHTWAVE_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLIGSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLIADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLICELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLISBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLIOPR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC12, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC16, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NORRIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ISIAUDIO_2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_LOAS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VODAFONE_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VODAFONE_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT24_SPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONICFOUNDRY_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INNINGS_TELECOM_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_SX8300P, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_SX5363S, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CUSEEME, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NTCSOFT_ALF2CM_ACM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DVM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DTS2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MAKEAVIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIVIO_MPEG4_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_ADAPTIVE_MULTIRATE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIVIO_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LEAD_SPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LEAD_VORBIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WAVPACK_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_1_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_2_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_3_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_3COM_NBX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FAAD_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AMR_NB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AMR_WP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_AMR_CBR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_AMR_VBR_SID, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_AVQSBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_SBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYMBOL_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOICEAGE_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INGENIENT_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG4_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ENCORE_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ZOLL_ASAO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SPEEX_VOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VIANIX_MASC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WM9_SPECTRUM_ANALYZER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMF_SPECTRUM_ANAYZER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_620, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_660, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_690, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_ADAPTIVE_MULTIRATE_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G722, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_SIREN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GLOBAL_IP_ILBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RADIOTIME_TIME_SHIFT_RADIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NICE_ACA, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NICE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G721, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G722_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_LBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NICE_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FRACE_TELECOM_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CODIAN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FLAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_EXTENSIBLE, ACE_TEXT_ALWAYS_CHAR ("EXTENSIBLE")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DEVELOPMENT, ACE_TEXT_ALWAYS_CHAR ("DEVELOPMENT")));

  // ---------------------------------------------------------------------------

  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ANALOG, ACE_TEXT_ALWAYS_CHAR ("Analog")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_DRM, ACE_TEXT_ALWAYS_CHAR ("DRM")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ALAW, ACE_TEXT_ALWAYS_CHAR ("ALAW")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MULAW, ACE_TEXT_ALWAYS_CHAR ("MULAW")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("ADPCM")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEG, ACE_TEXT_ALWAYS_CHAR ("MPEG")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL, ACE_TEXT_ALWAYS_CHAR ("DOLBY_DIGITAL")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_WMA_PRO, ACE_TEXT_ALWAYS_CHAR ("WMA_PRO")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DTS, ACE_TEXT_ALWAYS_CHAR ("DTS")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG1, ACE_TEXT_ALWAYS_CHAR ("MPEG1")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG2, ACE_TEXT_ALWAYS_CHAR ("MPEG2")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG3, ACE_TEXT_ALWAYS_CHAR ("MPEG3")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_AAC, ACE_TEXT_ALWAYS_CHAR ("AAC")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_ATRAC, ACE_TEXT_ALWAYS_CHAR ("ATRAC")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_ONE_BIT_AUDIO, ACE_TEXT_ALWAYS_CHAR ("ONE_BIT_AUDIO")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS, ACE_TEXT_ALWAYS_CHAR ("DOLBY_DIGITAL_PLUS")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD, ACE_TEXT_ALWAYS_CHAR ("DTS_HD")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP, ACE_TEXT_ALWAYS_CHAR ("DOLBY_MLP")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DST, ACE_TEXT_ALWAYS_CHAR ("DST")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEGLAYER3, ACE_TEXT_ALWAYS_CHAR ("MPEGLAYER3")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR ("HEAAC")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO2, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO2")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO3, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO3")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO_LOSSLESS")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_DTS_AUDIO, ACE_TEXT_ALWAYS_CHAR ("DTS_AUDIO")));
  Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_SDDS_AUDIO, ACE_TEXT_ALWAYS_CHAR ("SDDS_AUDIO")));

  if (likely (coInitialize_in))
  {
    HRESULT result = CoInitializeEx (NULL,
                                     (COINIT_MULTITHREADED    |
                                      COINIT_DISABLE_OLE1DDE  |
                                      COINIT_SPEED_OVER_MEMORY));
    if (unlikely (FAILED (result))) // 0x80010106: RPC_E_CHANGED_MODE
    {
      if (result != RPC_E_CHANGED_MODE) // already initialized
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
        return false;
      } // end IF
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    } // end IF
  } // end IF

  return true;
}

void
Stream_MediaFramework_DirectShow_Tools::finalize (bool coUninitialize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::finalize"));

  if (likely (coUninitialize_in))
    CoUninitialize ();
}

bool
Stream_MediaFramework_DirectShow_Tools::addToROT (IFilterGraph* filterGraph_in,
                                                  DWORD& ID_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::addToROT"));

  // initialize return value(s)
  ID_out = 0;

  // sanity check(s)
  ACE_ASSERT (filterGraph_in);

  IUnknown* iunknown_p = filterGraph_in;
  IRunningObjectTable* ROT_p = NULL;
  IMoniker* moniker_p = NULL;
  OLECHAR buffer_a[BUFSIZ];
  LPCOLESTR lpszDelim = OLESTR ("!");
  LPCOLESTR pszFormat = OLESTR ("FilterGraph %08x pid %08x");

  HRESULT result = GetRunningObjectTable (0, &ROT_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to GetRunningObjectTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (ROT_p);

  // *IMPORTANT NOTE*: do not change this syntax, otherwise graphedt.exe
  //                   cannot find the graph
  result =
#if defined (_WIN32) && !defined (OLE2ANSI) // see <WTypes.h>
//    ::StringCchPrintf (buffer_a, NUMELMS (buffer_a),
    ::StringCchPrintfW (buffer_a, sizeof (OLECHAR[BUFSIZ]) / sizeof ((buffer_a)[0]),
#else
    ::StringCchPrintfA (buffer_a, sizeof (OLECHAR[BUFSIZ]) / sizeof ((buffer_a)[0]),
#endif // _WIN32 && !OLE2ANSI
                        pszFormat,
                        (DWORD_PTR)iunknown_p, ACE_OS::getpid ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to StringCchPrintf(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF

  result = CreateItemMoniker (lpszDelim, buffer_a,
                              &moniker_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateItemMoniker(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF

  // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
  // to the object.  Using this flag will cause the object to remain
  // registered until it is explicitly revoked with the Revoke() method.
  // Not using this flag means that if GraphEdit remotely connects
  // to this graph and then GraphEdit exits, this object registration
  // will be deleted, causing future attempts by GraphEdit to fail until
  // this application is restarted or until the graph is registered again.
  result = ROT_p->Register (ROTFLAGS_REGISTRATIONKEEPSALIVE,
                            iunknown_p,
                            moniker_p,
                            &ID_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRunningObjectTable::Register(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered filter graph in running object table (ID: %d)\n"),
              ID_out));

  moniker_p->Release ();
  ROT_p->Release ();

  return true;

error:
  if (moniker_p)
    moniker_p->Release ();
  if (ROT_p)
    ROT_p->Release ();

  return false;
}
bool
Stream_MediaFramework_DirectShow_Tools::removeFromROT (DWORD id_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::removeFromROT"));

  // sanity check(s)
  ACE_ASSERT (id_in);

  IRunningObjectTable* ROT_p = NULL;
  HRESULT result = GetRunningObjectTable (0, &ROT_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to GetRunningObjectTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (ROT_p);

  result = ROT_p->Revoke (id_in);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d): \"%s\", continuing\n"),
                id_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed filter graph from running object table (id was: %d)\n"),
                id_in));

  ROT_p->Release (); ROT_p = NULL;

  return true;
} // end IF

void
Stream_MediaFramework_DirectShow_Tools::debug (IGraphBuilder* builder_in,
                                               const std::string& fileName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::debug"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (Stream_MediaFramework_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE)
    goto continue_;

  if (!fileName_in.empty ())
  {
    Stream_MediaFramework_DirectShow_Tools::logFileHandle =
      ACE_TEXT_CreateFile (ACE_TEXT (fileName_in.c_str ()),
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           CREATE_ALWAYS, // TRUNCATE_EXISTING :-)
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (Stream_MediaFramework_DirectShow_Tools::logFileHandle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CreateFile(\"%s\"): \"%s\", returning\n"),
                  ACE_TEXT (fileName_in.c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false).c_str ())));
      return;
    } // end IF
  } // end IF

continue_:
  HRESULT result =
    builder_in->SetLogFile (((Stream_MediaFramework_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE) ? reinterpret_cast<DWORD_PTR> (Stream_MediaFramework_DirectShow_Tools::logFileHandle)
                                                                                                           : NULL));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::SetLogFile(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT (fileName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

    // clean up
    if (!CloseHandle (Stream_MediaFramework_DirectShow_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false).c_str ())));

    return;
  } // end IF

  if (fileName_in.empty () &&
      (Stream_MediaFramework_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE))
  {
    if (!CloseHandle (Stream_MediaFramework_DirectShow_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false).c_str ())));
  } // end IF
}

void
Stream_MediaFramework_DirectShow_Tools::dump (const Stream_MediaFramework_DirectShow_Graph_t& graphLayout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::dump"));

  bool is_first = true;
  std::string graph_layout_string;

  for (Stream_MediaFramework_DirectShow_GraphConstIterator_t iterator = graphLayout_in.begin ();
       iterator != graphLayout_in.end ();
       ++iterator)
  {
    if (is_first)
      is_first = false;
    else
      graph_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

    graph_layout_string += ACE_TEXT_ALWAYS_CHAR ("\"");
    graph_layout_string += ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ());
    graph_layout_string += ACE_TEXT_ALWAYS_CHAR ("\"");
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (graph_layout_string.c_str ())));
}
void
Stream_MediaFramework_DirectShow_Tools::dump (const Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::dump"));

  std::string graph_layout_string = ACE_TEXT_ALWAYS_CHAR ("[");
  Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator =
    graphConfiguration_in.begin ();

  graph_layout_string +=
    Stream_MediaFramework_Tools::mediaSubTypeToString ((*iterator).mediaType->subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
  graph_layout_string += ACE_TEXT_ALWAYS_CHAR ("] --> \"");
  graph_layout_string +=
    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ());
  graph_layout_string += ACE_TEXT_ALWAYS_CHAR ("\"");

  for (++iterator;
       iterator != graphConfiguration_in.end ();
       ++iterator)
  {
    graph_layout_string += ACE_TEXT_ALWAYS_CHAR (" -- ");
    graph_layout_string +=
      ((*iterator).mediaType ? Stream_MediaFramework_Tools::mediaSubTypeToString ((*iterator).mediaType->subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW)
                             : std::string (ACE_TEXT_ALWAYS_CHAR ("NULL")));
    graph_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> \"");
    graph_layout_string +=
      ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ());
    graph_layout_string += ACE_TEXT_ALWAYS_CHAR ("\"");
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (graph_layout_string.c_str ())));
}
void
Stream_MediaFramework_DirectShow_Tools::dump (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (pin_in);

  IBaseFilter* filter_p =
    Stream_MediaFramework_DirectShow_Tools::toFilter (pin_in);
  ACE_ASSERT (filter_p);
  std::string filter_name_string =
    Stream_MediaFramework_DirectShow_Tools::name (filter_p);
  filter_p->Release (); filter_p = NULL;

  IEnumMediaTypes* ienum_media_types_p = NULL;
  HRESULT result = pin_in->EnumMediaTypes (&ienum_media_types_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::EnumMediaTypes(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
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
  { ACE_ASSERT (media_types_a[0]);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s:%s[#%d]: %s\n"),
                ACE_TEXT (filter_name_string.c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_in).c_str ()),
                index,
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*media_types_a[0], true).c_str ())));

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_types_a[0]);
    ++index;
  } // end WHILE
  ienum_media_types_p->Release (); ienum_media_types_p = NULL;
}
void
Stream_MediaFramework_DirectShow_Tools::dump (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::dump"));

  LONG width = -1;
  LONG height = -1;

  // --> audio
  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
    ACE_ASSERT (waveformatex_p);

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\" [rate/resolution/channels]: %d,%d,%d\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mediaType_in.subtype).c_str ()),
                waveformatex_p->nSamplesPerSec,
                waveformatex_p->wBitsPerSample,
                waveformatex_p->nChannels));
    
    return;
  } // end IF
  // --> video
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    ACE_ASSERT (video_info_header_p);

    width = video_info_header_p->bmiHeader.biWidth;
    height = video_info_header_p->bmiHeader.biHeight;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    ACE_ASSERT (video_info_header_p);

    width = video_info_header_p->bmiHeader.biWidth;
    height = video_info_header_p->bmiHeader.biHeight;
  } // end ELSE
  else if (!InlineIsEqualGUID (mediaType_in.formattype, GUID_NULL)) // <-- 'don't care'
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return;
  } // end ELSE

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\" - \"%s\": %dx%d\n"),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mediaType_in.subtype).c_str ()),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ()),
              width, height));
}

std::string
Stream_MediaFramework_DirectShow_Tools::name (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::name"));

  std::string result;

  // sanity check(s)
  ACE_ASSERT (pin_in);

  struct _PinInfo pin_info;
  ACE_OS::memset (&pin_info, 0, sizeof (struct _PinInfo));
  HRESULT result_2 = pin_in->QueryPinInfo (&pin_info);
  ACE_ASSERT (SUCCEEDED (result_2));
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (pin_info.achName));

  return result;
}

IPin*
Stream_MediaFramework_DirectShow_Tools::pin (IBaseFilter* filter_in,
                                             enum _PinDirection direction_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::pin"));

  IPin* result = NULL;

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IEnumPins* enumerator_p = NULL;
  HRESULT result_2 = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (enumerator_p);

  //IKsPropertySet* property_set_p = NULL;
  //struct _GUID GUID_s = GUID_NULL;
  enum _PinDirection pin_direction;
  while (S_OK == enumerator_p->Next (1, &result, NULL))
  {
    ACE_ASSERT (result);

    result_2 = result->QueryDirection (&pin_direction);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));

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
    //    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
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
    //    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
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
    //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ()),
    //            ((direction_in == PINDIR_INPUT) ? ACE_TEXT ("input") : ACE_TEXT ("output"))));
    return NULL;
  } // end IF

  return result;
}

IPin*
Stream_MediaFramework_DirectShow_Tools::capturePin (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::capturePin"));

  IPin* result = NULL;

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IEnumPins* enumerator_p = NULL;
  enum _PinDirection pin_direction;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
  DWORD returned_size = 0;
  IAMStreamConfig* stream_config_p = NULL;

  HRESULT result_2 = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);

  while (enumerator_p->Next (1, &result, NULL) == S_OK)
  { ACE_ASSERT (result);
    result_2 = result->QueryDirection (&pin_direction);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      result->Release (); result = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
    if (pin_direction != PINDIR_OUTPUT)
    {
      result->Release (); result = NULL;
      continue;
    } // end IF
    result_2 = result->QueryInterface (IID_IKsPropertySet,
                                       (void**)&property_set_p);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryInterface(IID_IKsPropertySet): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      result->Release (); result = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
    ACE_ASSERT (property_set_p);
    result_2 = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                                    NULL, 0,
                                    &GUID_s, sizeof (struct _GUID), &returned_size);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      property_set_p->Release ();
      result->Release (); result = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (struct _GUID));
    property_set_p->Release (); property_set_p = NULL;
    if (InlineIsEqualGUID (GUID_s, PIN_CATEGORY_CAPTURE))
      break; // found capture pin
    result->Release (); result = NULL;
  } // end WHILE
  enumerator_p->Release (); enumerator_p = NULL;

  return result;
}

struct _AMMediaType*
Stream_MediaFramework_DirectShow_Tools::defaultCaptureFormat (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::defaultCaptureFormat"));

  struct _AMMediaType* result_p = NULL;

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::capturePin (filter_in);
  ACE_ASSERT (pin_p);
  if (!Stream_MediaFramework_DirectShow_Tools::getFirstFormat (pin_p,
                                                               GUID_NULL,
                                                               result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::getFirstFormat(\"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));
    pin_p->Release (); pin_p = NULL;
    return NULL;
  } // end IF
  ACE_ASSERT (result_p);
  pin_p->Release (); pin_p = NULL;

  return result_p;
}

IBaseFilter*
Stream_MediaFramework_DirectShow_Tools::next (IBaseFilter* filter_in)
{
  IBaseFilter* result = NULL;

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IPin* pin_p = NULL;
  enum _PinDirection pin_direction_e;
  IPin* pin_2 = NULL;
  IEnumPins* enumerator_p = NULL;
  HRESULT result_2 = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (enumerator_p);
  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
  { ACE_ASSERT (pin_p);
    result_2 = pin_p->QueryDirection (&pin_direction_e);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      pin_p->Release (); pin_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return NULL;
    } // end IF
    if (pin_direction_e != PINDIR_OUTPUT)
    {
      pin_p->Release (); pin_p = NULL;
      continue;
    } // end IF
    result_2 = pin_p->ConnectedTo (&pin_2);
    if (FAILED (result_2))
    {
      pin_p->Release (); pin_p = NULL;
      continue;
    } // end IF
    pin_p->Release (); pin_p = NULL;
    break;
  } // end WHILE
  enumerator_p->Release (); enumerator_p = NULL;
  if (likely (pin_2))
  {
    result = Stream_MediaFramework_DirectShow_Tools::toFilter (pin_2);
    ACE_ASSERT (result);
    pin_2->Release (); pin_2 = NULL;
  } // end IF

  return result;
}

IBaseFilter*
Stream_MediaFramework_DirectShow_Tools::toFilter (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toFilter"));

  // sanity check(s)
  ACE_ASSERT (pin_in);

  struct _PinInfo pin_info;
  ACE_OS::memset (&pin_info, 0, sizeof (struct _PinInfo));
  HRESULT result = pin_in->QueryPinInfo (&pin_info);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryPinInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (pin_info.pFilter);

  return pin_info.pFilter;
}

std::string
Stream_MediaFramework_DirectShow_Tools::name (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::name"));

  std::string result;

  struct _FilterInfo filter_info;
  ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
  HRESULT result_2 = filter_in->QueryFilterInfo (&filter_info);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
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
Stream_MediaFramework_DirectShow_Tools::hasPropertyPages (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::hasPropertyPages"));

  // sanity check(s)
  ACE_ASSERT (filter_in);
  
  ISpecifyPropertyPages* property_pages_p = NULL;
  HRESULT result = filter_in->QueryInterface (IID_PPV_ARGS (&property_pages_p));
  if (property_pages_p)
  {
    property_pages_p->Release (); property_pages_p = NULL;
  } // end IF

  return SUCCEEDED (result);
}

bool
Stream_MediaFramework_DirectShow_Tools::loadSourceGraph (IBaseFilter* sourceFilter_in,
                                                         const std::wstring& sourceFilterName_in,
                                                         IGraphBuilder*& IGraphBuilder_inout,
                                                         IAMBufferNegotiation*& IAMBufferNegotiation_out,
                                                         IAMStreamConfig*& IAMStreamConfig_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::loadSourceGraph"));

  // initialize return value(s)
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

  // sanity check(s)
  ACE_ASSERT (sourceFilter_in);

  bool release_builder = false;
  HRESULT result = E_FAIL;
  struct _GUID GUID_s = GUID_NULL;

  if (!IGraphBuilder_inout)
  {
    release_builder = true;
    ICaptureGraphBuilder2* builder_2 = NULL;
    result =
      CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&builder_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_CaptureGraphBuilder2): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (builder_2);

    result = CoCreateInstance (CLSID_FilterGraph, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&IGraphBuilder_inout));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
      builder_2->Release (); builder_2 = NULL;
      return false;
    } // end IF
    ACE_ASSERT (IGraphBuilder_inout);

    result = builder_2->SetFiltergraph (IGraphBuilder_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ICaptureGraphBuilder2::SetFiltergraph(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      builder_2->Release (); builder_2 = NULL;
      goto error;
    } // end IF
    builder_2->Release (); builder_2 = NULL;
  } // end IF
  else
  {
    if (!Stream_MediaFramework_DirectShow_Tools::clear (IGraphBuilder_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::clear(), aborting\n")));
      return false;
    } // end IF
  } // end ELSE
  ACE_ASSERT (IGraphBuilder_inout);

  result =
    IGraphBuilder_inout->AddFilter (sourceFilter_in,
                                    sourceFilterName_in.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(0x%@: \"%s\"): \"%s\", aborting\n"),
                sourceFilter_in,
                ACE_TEXT (ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ())),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added \"%s\"\n"),
  //            ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ())));

  IEnumPins* enumerator_p = NULL;
  IPin* pin_p, *pin_2 = NULL;
  IKsPropertySet* property_set_p = NULL;
  DWORD returned_size = 0;

  result = sourceFilter_in->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (enumerator_p);

  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
  {
    ACE_ASSERT (pin_p);

    property_set_p = NULL;
    //result = pin_p->QueryInterface (IID_PPV_ARGS (&property_set_p));
    result = pin_p->QueryInterface (IID_IKsPropertySet,
                                    (void**)&property_set_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryInterface(IID_IKsPropertySet): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      pin_p->Release (); pin_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (property_set_p);
    result =
      property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                           NULL, 0,
                           &GUID_s, sizeof (struct _GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      property_set_p->Release (); property_set_p = NULL;
      pin_p->Release (); pin_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (struct _GUID));
    property_set_p->Release ();

    if (InlineIsEqualGUID (GUID_s, PIN_CATEGORY_CAPTURE))
    {
      pin_2 = pin_p;
      break;
    } // end IF

    pin_p->Release (); pin_p = NULL;
  } // end WHILE
  enumerator_p->Release (); enumerator_p = NULL;
  if (!pin_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\" [0x%@]: no capture pin found, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ()),
                sourceFilter_in));
    goto error;
  } // end IF

  result = pin_2->QueryInterface (IID_PPV_ARGS (&IAMBufferNegotiation_out));
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMBufferNegotiation): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

  result = pin_2->QueryInterface (IID_PPV_ARGS (&IAMStreamConfig_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_2->Release (); pin_2 = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (IAMStreamConfig_out);
  pin_2->Release (); pin_2 = NULL;

  return true;

error:
  if (release_builder &&
      IGraphBuilder_inout)
  {
    IGraphBuilder_inout->Release (); IGraphBuilder_inout = NULL;
  } // end IF
  if (IAMBufferNegotiation_out)
  {
    IAMBufferNegotiation_out->Release (); IAMBufferNegotiation_out = NULL;
  } // end IF
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
  } // end IF

  return false;
}

bool
Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder* builder_in,
                                                 const Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::connect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!graphConfiguration_in.empty ());

  IBaseFilter* filter_p = NULL;
  Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator =
    graphConfiguration_in.begin ();
  HRESULT result =
    builder_in->FindFilterByName ((*iterator).filterName.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                             PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    filter_p->Release (); filter_p = NULL;
    return false;
  } // end IF
  filter_p->Release (); filter_p = NULL;

  IAMStreamConfig* stream_config_p = NULL;
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IPin::QueryInterface(IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_p->Release (); pin_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (stream_config_p);
  result =
    stream_config_p->SetFormat ((*iterator).mediaType); // *NOTE*: 'NULL' should reset the pin
  if (FAILED (result))
  {
    if ((*iterator).mediaType)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IAMStreamConfig::SetFormat(): \"%s\" (media type was: %s), continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*(*iterator).mediaType, true).c_str ())));
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IAMStreamConfig::SetFormat(NULL): \"%s\", continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
  } // end IF
  stream_config_p->Release (); stream_config_p = NULL;

  IPin* pin_2 = NULL, *pin_3 = NULL;
  Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator_2 =
    iterator;
  for (++iterator;
       iterator != graphConfiguration_in.end ();
       ++iterator_2)
  { ACE_ASSERT (pin_p); ACE_ASSERT (!filter_p);
    result = builder_in->FindFilterByName ((*iterator).filterName.c_str (),
                                           &filter_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      pin_p->Release (); pin_p = NULL;
      return false;
    } // end IF
    ACE_ASSERT (filter_p);
    pin_2 = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                         PINDIR_INPUT);
    if (!pin_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: has no input pin, aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
      filter_p->Release (); filter_p = NULL;
      return false;
    } // end IF

    result =
      ((*iterator).connectDirect ? builder_in->ConnectDirect (pin_p,
                                                              pin_2,
                                                              (*iterator).mediaType)
                                 : pin_p->Connect (pin_2,
                                                   (*iterator).mediaType));
    if (FAILED (result)) // 0x80040200: VFW_E_INVALIDMEDIATYPE
                         // 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
                         // 0x80040217: VFW_E_CANNOT_CONNECT
                         // 0x8004022A: VFW_E_TYPE_NOT_ACCEPTED
                         // 0x80040255: VFW_E_NO_DECOMPRESSOR
                         // 0x80070057: 
    {
      // *TODO*: evidently, some filters do not expose their preferred media
      //         types (e.g. AVI Splitter) [until the filter is connected], so
      //         the straight-forward, 'direct' pin connection algorithm will
      //         not always work. Note how (such as in this example), this
      //         actually makes some sense, as 'container'- or other 'meta-'
      //         filters sometimes actually do not know (or care) about what
      //         kind of data they contain
      // *NOTE*: 'fixing' this requires some in-depth knowledge about
      //         _AMMediaType (in-)compatibilities, and other inner workings of
      //         DirectShow (such as what the algorithm is that IGraphBuilder
      //         uses to intelligently retrieve 'pin'-compatible media types)...
#if defined (_DEBUG)
      if ((result == VFW_E_INVALIDMEDIATYPE)    ||
          (result == VFW_E_NO_ACCEPTABLE_TYPES) ||
          (result == VFW_E_TYPE_NOT_ACCEPTED))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to connect \"%s\" to \"%s\": \"%s\" (0x%x), dumping pins\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()), result));
        Stream_MediaFramework_DirectShow_Tools::dump (pin_p);
        Stream_MediaFramework_DirectShow_Tools::dump (pin_2);
      } // end IF
#endif // _DEBUG
      //else
      //{
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("'direct' pin connection %s/%s <--> %s/%s failed (media type was: %s): \"%s\" (0x%x), retrying...\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                    ((*iterator).mediaType ? ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*(*iterator).mediaType, true).c_str ())
                                           : ACE_TEXT ("NULL")),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()), result));
      //} // end ELSE

      result = builder_in->Connect (pin_p, pin_2);
      if (FAILED (result)) // 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
                           // 0x80040217: VFW_E_CANNOT_CONNECT
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("'intelligent' pin connection %s/%s <--> %s/%s failed: \"%s\" (0x%x), aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()),
                    result));
//#if defined (_DEBUG)
//        Stream_MediaFramework_DirectShow_Tools::countFormats (pin_2,
//                                                              GUID_NULL);
//#endif // _DEBUG
      } // end IF
      else
        goto continue_;
      pin_2->Release (); pin_2 = NULL;
      pin_p->Release (); pin_p = NULL;
      filter_p->Release (); filter_p = NULL;
      return false;
    } // end IF
continue_:
#if defined (_DEBUG)
    struct _AMMediaType media_type_s =
      Stream_MediaFramework_DirectShow_Tools::toFormat (pin_p);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connected \"%s\" to \"%s\": %s\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (media_type_s, true).c_str ())));
    Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
    ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#endif // _DEBUG
//continue_2:
    pin_2->Release (); pin_2 = NULL;
    pin_p->Release (); pin_p = NULL;

    if (++iterator != graphConfiguration_in.end ())
    { ACE_ASSERT (!pin_p);
      pin_p =
        Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                     PINDIR_OUTPUT);
      if (!pin_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: has no output pin, aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
        filter_p->Release (); filter_p = NULL;
        break;
      } // end IF
    } // end IF
    filter_p->Release (); filter_p = NULL;
  } // end FOR

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder* builder_in,
                                                 IBaseFilter* filter_in,
                                                 IBaseFilter* filter2_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::connectFirst"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (filter_in);
  ACE_ASSERT (filter2_in);

  IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_in,
                                                             PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));
    return false;
  } // end IF
  IPin* pin_2 = Stream_MediaFramework_DirectShow_Tools::pin (filter2_in,
                                                             PINDIR_INPUT);
  if (!pin_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no input pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter2_in).c_str ())));
    pin_p->Release (); pin_p = NULL;
    return false;
  } // end IF

  HRESULT result = builder_in->Connect (pin_p,
                                        pin_2);
  if (FAILED (result)) // 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
                        // 0x80040217: VFW_E_CANNOT_CONNECT
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("'intelligent' pin connection %s/%s <--> %s/%s failed: \"%s\" (0x%x), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_2).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter2_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()),
                result));
    pin_p->Release (); pin_p = NULL;
    pin_2->Release (); pin_2 = NULL;
    return false;
  } // end IF
  pin_p->Release (); pin_p = NULL;
  pin_2->Release (); pin_2 = NULL;

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::connectFirst (IGraphBuilder* builder_in,
                                                      const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::connectFirst"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::pin(PINDIR_OUTPUT), aborting\n")));
    filter_p->Release (); filter_p = NULL;
    return false;
  } // end IF

  IPin* pin_2 = NULL;
loop:
  result = pin_p->ConnectedTo (&pin_2);
  if (FAILED (result))
  {
    filter_p = Stream_MediaFramework_DirectShow_Tools::toFilter (pin_p);
    ACE_ASSERT (filter_p);
    result = builder_in->Render (pin_p);
    if (FAILED (result))
    {

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::Render(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      filter_p->Release (); filter_p = NULL;
      pin_p->Release (); pin_p = NULL;
      return false;
    } // end IF
    filter_p->Release (); filter_p = NULL;

    return true;
  } // end IF
  ACE_ASSERT (pin_2);
  pin_p->Release (); pin_p = NULL;

  filter_p = Stream_MediaFramework_DirectShow_Tools::toFilter (pin_2);
  if (!filter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::toFilter(0x%@), aborting\n"),
                pin_2));
    pin_2->Release (); pin_2 = NULL;
    return false;
  } // end IF
  pin_2->Release (); pin_2 = NULL;

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    filter_p->Release (); filter_p = NULL;
    return true; // filter has no output pin --> sink
  } // end IF
  filter_p->Release (); filter_p = NULL;

  goto loop;

  ACE_NOTREACHED (return false;)
}
bool
Stream_MediaFramework_DirectShow_Tools::connected (IGraphBuilder* builder_in,
                                                   const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::connected"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::pin(PINDIR_OUTPUT), aborting\n")));

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

  filter_p = Stream_MediaFramework_DirectShow_Tools::toFilter (pin_2);
  if (!filter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::toFilter(0x%@), aborting\n"),
                pin_2));

    // clean up
    pin_2->Release ();

    return false;
  } // end IF
  pin_2->Release ();
  pin_2 = NULL;

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
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
Stream_MediaFramework_DirectShow_Tools::graphBuilderConnect (IGraphBuilder* builder_in,
                                                             const Stream_MediaFramework_DirectShow_Graph_t& graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::graphBuilderConnect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!graph_in.empty ());

  IBaseFilter* filter_p = NULL;
  Stream_MediaFramework_DirectShow_GraphConstIterator_t iterator = graph_in.begin ();
  HRESULT result =
    builder_in->FindFilterByName ((*iterator).c_str (),
      &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
      //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
      //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
  Stream_MediaFramework_DirectShow_GraphConstIterator_t iterator_2;
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
      //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
        Stream_MediaFramework_DirectShow_Tools::dump (pin_p);
        // *TODO*: evidently, some filters do not expose their preferred media
        //         types (e.g. AVI Splitter), so the straight-forward, 'direct'
        //         pin connection algorithm (as implemented here) will not
        //         always work. Note how (such as in this example), this
        //         actually makes some sense, as 'container'- or other 'meta-'
        //         filters sometimes actually do not know (or care) about what
        //         kind of data they contain
        Stream_MediaFramework_DirectShow_Tools::dump (pin_2);
      } // end IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGraphBuilder::Connect() \"%s\" to \"%s\": \"%s\" (0x%x), aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*--iterator_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()),
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

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
Stream_MediaFramework_DirectShow_Tools::append (IGraphBuilder* builder_in,
                                                IBaseFilter* filter_in,
                                                const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::append"));

    // sanity check(s)
  ACE_ASSERT (builder_in);

  // find trailing (connected) filter
  IBaseFilter* prev_p = NULL;
  IEnumFilters* enumerator_p = NULL;
  HRESULT result = builder_in->EnumFilters (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  IBaseFilter* filter_p = NULL;
  IBaseFilter* filter_2 = NULL;
//next:
  while (S_OK == enumerator_p->Next (1, &filter_p, NULL))
  { ACE_ASSERT (filter_p);
    break;
  } // end WHILE
  enumerator_p->Release (); enumerator_p = NULL;
  while (filter_p)
  {
    filter_2 = Stream_MediaFramework_DirectShow_Tools::next (filter_p);
    if (filter_2)
    {
      filter_p->Release (); filter_p = NULL;
      filter_p = filter_2;
      continue;
    } // end IF
    break;
  } // end WHILE
  if (unlikely (!filter_p))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no trailing filter found, adding \"%s\"\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("found trailing filter (was: \"%s\"), continuing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));

  result = builder_in->AddFilter (filter_in,
                                  filterName_in.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    if (filter_p) filter_p->Release ();
    return false;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ())));
#endif // _DEBUG

  if (filter_p)
  {
    if (!Stream_MediaFramework_DirectShow_Tools::connect (builder_in,
                                                          filter_p,
                                                          filter_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::connect(\"%s\",\"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                  ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ())));
      filter_p->Release (); filter_p = NULL;
      return false;
    } // end IF
    filter_p->Release (); filter_p = NULL;
  } // end IF

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::clear (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::clear"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IEnumFilters* enumerator_p = NULL;
  HRESULT result = builder_in->EnumFilters (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  IBaseFilter* filter_p = NULL;
#if defined (_DEBUG)
  struct _FilterInfo filter_info;
#endif // _DEBUG
  while (enumerator_p->Next (1, &filter_p, NULL) == S_OK)
  { ACE_ASSERT (filter_p);
#if defined (_DEBUG)
    ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
    result = filter_p->QueryFilterInfo (&filter_info);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      filter_p->Release (); filter_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
    if (filter_info.pGraph)
      filter_info.pGraph->Release ();
#endif // _DEBUG
    result = builder_in->RemoveFilter (filter_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGrapBuilder::RemoveFilter(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      filter_p->Release (); filter_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
    filter_p->Release (); filter_p = NULL;
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName)));
#endif // _DEBUG

    result = enumerator_p->Reset ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IEnumFilters::Reset(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
  } // end WHILE
  enumerator_p->Release (); enumerator_p = NULL;

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::disconnect (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IEnumPins* enumerator_p = NULL;
  IPin* pin_p = NULL, *pin_2 = NULL;
  HRESULT result = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);

  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
  { ACE_ASSERT (pin_p);
    pin_2 = NULL;
    result = pin_p->ConnectedTo (&pin_2);
    if (FAILED (result))
    {
      pin_p->Release (); pin_p = NULL;
      continue;
    } // end IF
    ACE_ASSERT (pin_2);

    result = pin_2->Disconnect ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::Disconnect(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      pin_2->Release (); pin_2 = NULL;
      pin_p->Release (); pin_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
    pin_2->Release (); pin_2 = NULL;

    result = pin_p->Disconnect ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::Disconnect(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      pin_p->Release (); pin_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("disconnected \"%s\"...\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));
#endif // _DEBUG
    pin_p->Release (); pin_p = NULL;
  } // end WHILE
  enumerator_p->Release (); enumerator_p = NULL;

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::remove (IGraphBuilder* builder_in,
                                                IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::remove"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (filter_in);
  ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::has (builder_in, ACE_TEXT_ALWAYS_WCHAR (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));

  if (!Stream_MediaFramework_DirectShow_Tools::disconnect (filter_in))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(%s), continuing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));

  HRESULT result = builder_in->RemoveFilter (filter_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGrapBuilder::RemoveFilter(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("removed \"%s\"...\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));
#endif // _DEBUG

  return true;
}
  
bool
Stream_MediaFramework_DirectShow_Tools::disconnect (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IEnumFilters* enumerator_p = NULL;
  HRESULT result = builder_in->EnumFilters (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  IBaseFilter* filter_p = NULL;
  while (S_OK == enumerator_p->Next (1, &filter_p, NULL))
  { ACE_ASSERT (filter_p);
    if (!Stream_MediaFramework_DirectShow_Tools::disconnect (filter_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(%s), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
      filter_p->Release (); filter_p = NULL;
      enumerator_p->Release (); enumerator_p = NULL;
      return false;
    } // end IF
    filter_p->Release (); filter_p = NULL;
  } // end WHILE
  enumerator_p->Release (); enumerator_p = NULL;

  return true;
}

void
Stream_MediaFramework_DirectShow_Tools::get (IGraphBuilder* builder_in,
                                             const std::wstring& filterName_in,
                                             Stream_MediaFramework_DirectShow_Graph_t& graphConfiguration_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::get"));

  // initialize return value(s)
  graphConfiguration_out.clear ();

  // sanity check(s)
  ACE_ASSERT (builder_in);

  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;
  IPin* pin_p, *pin_2 = NULL;

  result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return;
  } // end IF
  graphConfiguration_out.push_back (filterName_in);

  do
  {
    pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                         PINDIR_OUTPUT);
    if (!pin_p)
      break; // done
    result = pin_p->ConnectedTo (&pin_2);
    if (FAILED (result))
      break;
    pin_p->Release ();
    filter_p->Release ();
    filter_p = Stream_MediaFramework_DirectShow_Tools::toFilter (pin_2);
    if (!filter_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::toFilter(), returning\n")));
      break;
    } // end IF
    pin_2->Release ();
    graphConfiguration_out.push_back (ACE_TEXT_ALWAYS_WCHAR (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()));
  } while (true);

//clean:
  if (pin_p)
    pin_p->Release ();
  if (pin_2)
    pin_2->Release ();
  if (filter_p)
    filter_p->Release ();
}

bool
Stream_MediaFramework_DirectShow_Tools::has (IGraphBuilder* builder_in,
                                             const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::has"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  filter_p->Release (); filter_p = NULL;

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::has (const Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_in,
                                             const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::has"));

  for (Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator = graphConfiguration_in.begin ();
       iterator != graphConfiguration_in.end ();
       ++iterator)
    if (!ACE_OS::strcmp ((*iterator).filterName.c_str (),
                         filterName_in.c_str ()))
      return true;

  return false;
}

bool
Stream_MediaFramework_DirectShow_Tools::reset (IGraphBuilder* builder_in,
                                               REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::reset"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  std::wstring filter_name;
  IBaseFilter* filter_p = NULL;
  HRESULT result = E_FAIL;

  if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    filter_name = STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
    filter_name = STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else if (InlineIsEqualGUID (deviceCategory_in, GUID_NULL))
  { // retrieve the first filter that has no input pin
    IEnumFilters* enumerator_p = NULL;
    result = builder_in->EnumFilters (&enumerator_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      return false;
    } // end IF
    IPin* pin_p = NULL;
    struct _FilterInfo filter_info;
    while (enumerator_p->Next (1, &filter_p, NULL) == S_OK)
    {
      ACE_ASSERT (filter_p);

      pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                           PINDIR_INPUT);
      if (pin_p)
      {
        pin_p->Release ();
        filter_p->Release ();
        continue;
      } // end IF

      ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
      result = filter_p->QueryFilterInfo (&filter_info);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

        // clean up
        filter_p->Release ();
        enumerator_p->Release ();

        return false;
      } // end IF
      filter_name = filter_info.achName;

      // clean up
      if (filter_info.pGraph)
        filter_info.pGraph->Release ();

      break;
    } // end WHILE
    enumerator_p->Release ();
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return false;
  } // end ELSE

  if (!Stream_MediaFramework_DirectShow_Tools::disconnect (builder_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), aborting\n")));
    return false;
  } // end IF

  if (filter_name.empty ())
    goto continue_;

  result =
    builder_in->FindFilterByName (filter_name.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

continue_:
  if (!Stream_MediaFramework_DirectShow_Tools::clear (builder_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::clear(), aborting\n")));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF

  if (filter_name.empty ())
    goto continue_2;

  result = builder_in->AddFilter (filter_p,
                                  filter_name.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  filter_p->Release ();
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ())));
#endif // _DEBUG

continue_2:
  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation (IGraphBuilder* builder_in,
                                                              const std::wstring& filterName_in,
                                                              IAMBufferNegotiation*& IAMBufferNegotiation_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (IAMBufferNegotiation_out)
  {
    IAMBufferNegotiation_out->Release ();
    IAMBufferNegotiation_out = NULL;
  } // end IF

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                            PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::pin(\"%s\",PINDIR_OUTPUT), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  filter_p->Release ();

  result = pin_p->QueryInterface (IID_PPV_ARGS (&IAMBufferNegotiation_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMBufferNegotiation): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);
  pin_p->Release ();

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::getVideoWindow (IGraphBuilder* builder_in,
                                                        const std::wstring& filterName_in,
                                                        IMFVideoDisplayControl*& IMFVideoDisplayControl_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::getVideoWindow"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (IMFVideoDisplayControl_out)
  {
    IMFVideoDisplayControl_out->Release ();
    IMFVideoDisplayControl_out = NULL;
  } // end IF

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IMFGetService* service_p = NULL;
  result = filter_p->QueryInterface (IID_PPV_ARGS (&service_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IMFGetService): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (service_p);
  filter_p->Release ();
  result = service_p->GetService (MR_VIDEO_RENDER_SERVICE,
                                  IID_PPV_ARGS (&IMFVideoDisplayControl_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFGetService::GetService(IID_IMFVideoDisplayControl): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

    // clean up
    service_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (IMFVideoDisplayControl_out);
  service_p->Release ();

  return true;
}

std::string
Stream_MediaFramework_DirectShow_Tools::toString_2 (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toString_2"));

  std::string result;

  Stream_MediaFramework_GUIDToStringMapIterator_t iterator =
    Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.find (mediaType_in.majortype);
  result = ACE_TEXT_ALWAYS_CHAR ("(maj/sub/fmt): ");
  if (iterator == Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.end ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media majortype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.majortype).c_str ())));
    result += Common_Tools::GUIDToString (mediaType_in.majortype);
  } // end IF
  else
    result += (*iterator).second;
  result += ACE_TEXT_ALWAYS_CHAR ("/");
  result +=
    Stream_MediaFramework_Tools::mediaSubTypeToString (mediaType_in.subtype);
  result += ACE_TEXT_ALWAYS_CHAR ("/");
  iterator =
    Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.find (mediaType_in.formattype);
  if (iterator == Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.end ())
  {
    if (!InlineIsEqualGUID (mediaType_in.formattype, GUID_NULL)) // <-- 'don't care'
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.formattype).c_str ())));
    result += Common_Tools::GUIDToString (mediaType_in.formattype);
  } // end IF
  else
    result += (*iterator).second;

  result += ACE_TEXT_ALWAYS_CHAR (" || (fixed/comp/size): ");
  std::ostringstream converter;
  converter << mediaType_in.bFixedSizeSamples;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("/");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.bTemporalCompression;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("/");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.lSampleSize;
  result += converter.str ();

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;

    result += ACE_TEXT_ALWAYS_CHAR (" || rates (bit/error/frame): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->dwBitRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->dwBitErrorRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->AvgTimePerFrame;
    result += converter.str ();

    result +=
      ACE_TEXT_ALWAYS_CHAR (" || image (width/height/planes/bpp/compression/size): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biWidth;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biHeight;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biPlanes;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biBitCount;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    // *NOTE*: see also wingdi.h:902
    if (video_info_header_p->bmiHeader.biCompression <= BI_PNG)
    { // *TODO*: support toString() functionality here as well
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << video_info_header_p->bmiHeader.biCompression;
      result += converter.str ();
    } // end ELSE
    else
      result +=
        Stream_MediaFramework_Tools::FOURCCToString (video_info_header_p->bmiHeader.biCompression);
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biSizeImage;
    result += converter.str ();
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;

    result += ACE_TEXT_ALWAYS_CHAR (" || rates (bit/error/frame): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwBitRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwBitErrorRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->AvgTimePerFrame;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR (" || flags (interlace/copyprotection): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwInterlaceFlags;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwCopyProtectFlags;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR (" || aspect ratio (x/y): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwPictAspectRatioX;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwPictAspectRatioY;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR (" || control flags: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwControlFlags;
    result += converter.str ();

    result +=
      ACE_TEXT_ALWAYS_CHAR (" || image (width/height/planes/bpp/compression/size): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biWidth;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biHeight;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biPlanes;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biBitCount;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    // *NOTE*: see also wingdi.h:902
    if (video_info_header2_p->bmiHeader.biCompression <= BI_PNG)
    { // *TODO*: support toString() functionality here as well
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << video_info_header2_p->bmiHeader.biCompression;
      result += converter.str ();
    } // end ELSE
    else
      result +=
        Stream_MediaFramework_Tools::FOURCCToString (video_info_header2_p->bmiHeader.biCompression);
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biSizeImage;
    result += converter.str ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;

    result += ACE_TEXT_ALWAYS_CHAR (" || format: ");

    WORD_TO_STRING_MAP_ITERATOR_T iterator =
      Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.find (waveformatex_p->wFormatTag);
    if (iterator == Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown wave formattype (was: %d), aborting\n"),
                  waveformatex_p->wFormatTag));
      return std::string ();
    } // end IF
    result += (*iterator).second;

    result +=
      ACE_TEXT_ALWAYS_CHAR (" || samples (channels/rate/bps/align/bps/size): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nChannels;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nSamplesPerSec;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nAvgBytesPerSec;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nBlockAlign;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->wBitsPerSample;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("/");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->cbSize;
    result += converter.str ();

    if (waveformatex_p->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
      WAVEFORMATEXTENSIBLE* waveformatextensible_p =
        reinterpret_cast<WAVEFORMATEXTENSIBLE*> (mediaType_in.pbFormat);

      result += ACE_TEXT_ALWAYS_CHAR (" || extensible (spb/mask/sub): ");

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      // *TODO*: the second argument may not be entirely accurate
      if (Stream_MediaFramework_Tools::isCompressedAudio (mediaType_in.subtype,
                                                          STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
        converter << waveformatextensible_p->Samples.wSamplesPerBlock;
      else
        converter << waveformatextensible_p->Samples.wValidBitsPerSample;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR ("/");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter <<
        std::hex << waveformatextensible_p->dwChannelMask << std::dec;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR ("/");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator =
        Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.find (waveformatextensible_p->SubFormat);
      if (iterator == Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.end ())
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("invalid/unknown wave subformat (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
        result += Common_Tools::GUIDToString (waveformatextensible_p->SubFormat);
      } // end IF
      else
        result += (*iterator).second;
      result += ACE_TEXT_ALWAYS_CHAR ("\"");
    } // end IF
  } // end ELSE IF
  else if (!InlineIsEqualGUID (mediaType_in.formattype, GUID_NULL)) // <-- 'don't care'
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.formattype).c_str ())));

  return result;
}

struct _AMMediaType
Stream_MediaFramework_DirectShow_Tools::toFormat (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toFormat"));

  // initialize return value(s)
  struct _AMMediaType result_s;
  ACE_OS::memset (&result_s, 0, sizeof (struct _AMMediaType));

  // sanity check(s)
  ACE_ASSERT (pin_in);

  HRESULT result = pin_in->ConnectionMediaType (&result_s);
  if (FAILED (result))
  {
    IBaseFilter* filter_p =
      Stream_MediaFramework_DirectShow_Tools::toFilter (pin_in);
    ACE_ASSERT (filter_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IPin::ConnectionMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    filter_p->Release (); filter_p = NULL;
    Stream_MediaFramework_DirectShow_Tools::free (result_s);
    ACE_OS::memset (&result_s, 0, sizeof (struct _AMMediaType));
    return result_s;
  } // end IF

  return result_s;
}

bool
Stream_MediaFramework_DirectShow_Tools::getOutputFormat (IGraphBuilder* builder_in,
                                                         const std::wstring& filterName_in,
                                                         struct _AMMediaType& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!filterName_in.empty ());

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (mediaType_inout);
  ACE_OS::memset (&mediaType_inout, 0, sizeof (struct _AMMediaType));

  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;
  IPin* pin_p = NULL;

  result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  if (!ACE_OS::strcmp (filterName_in.c_str (),
                       STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB))
  {
    ISampleGrabber* isample_grabber_p = NULL;
    result = filter_p->QueryInterface (IID_ISampleGrabber,
                                       (void**)&isample_grabber_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (isample_grabber_p);

    // *NOTE*: connect()ing the 'sample grabber' to the 'null renderer' breaks
    //         the connection between the 'AVI decompressor' and the 'sample
    //         grabber' (go ahead, try it in with graphedit.exe)
    result = isample_grabber_p->GetConnectedMediaType (&mediaType_inout);
    if (FAILED (result)) // 0x80040209: VFW_E_NOT_CONNECTED
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ISampleGrabber::GetConnectedMediaType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    isample_grabber_p->Release (); isample_grabber_p = NULL;
    goto continue_;
  } // end IF

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (pin_p);
  mediaType_inout = Stream_MediaFramework_DirectShow_Tools::toFormat (pin_p);
  pin_p->Release (); pin_p = NULL;

continue_:
  filter_p->Release (); filter_p = NULL;

  return true;

error:
  Stream_MediaFramework_DirectShow_Tools::free (mediaType_inout);
  if (filter_p)
    filter_p->Release ();
  if (pin_p)
    pin_p->Release ();

  return false;
}

bool
Stream_MediaFramework_DirectShow_Tools::getFirstFormat (IPin* pin_in,
                                                        REFGUID mediaSubType_in,
                                                        struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::getFirstFormat"));

  // sanity check(s)
  ACE_ASSERT (pin_in);
  ACE_ASSERT (!mediaType_inout);

  IEnumMediaTypes* enumerator_p = NULL;
  HRESULT result = pin_in->EnumMediaTypes (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF

  struct _AMMediaType* media_types_a[1];
  ACE_OS::memset (media_types_a, 0, sizeof (media_types_a));
  ULONG fetched = 0;
  do
  {
    result = enumerator_p->Next (1,
                                 media_types_a,
                                 &fetched);
    if (FAILED (result) ||
        (result == S_FALSE)) // most probable reason: pin is not connected
      break;

    // sanity check(s)
    ACE_ASSERT (media_types_a[0]);

    if (InlineIsEqualGUID (mediaSubType_in, GUID_NULL) ||
        InlineIsEqualGUID (mediaSubType_in, media_types_a[0]->subtype))
      break;

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_types_a[0]);
  } while (true);
  enumerator_p->Release (); enumerator_p = NULL;

  if (media_types_a[0])
    mediaType_inout = media_types_a[0];

  return !!mediaType_inout;
}

bool
Stream_MediaFramework_DirectShow_Tools::hasUncompressedFormat (REFGUID deviceCategory_in,
                                                               IPin* pin_in,
                                                               struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::hasUncompressedFormat"));

  // sanity check(s)
  ACE_ASSERT (pin_in);
  ACE_ASSERT (!mediaType_inout);

  IEnumMediaTypes* enumerator_p = NULL;
  HRESULT result = pin_in->EnumMediaTypes (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF

  struct _AMMediaType* media_types_a[1];
  ACE_OS::memset (media_types_a, 0, sizeof (media_types_a));
  ULONG fetched = 0;
  do
  {
    result = enumerator_p->Next (1,
                                 media_types_a,
                                 &fetched);
    // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
    //         (TM) AVI/MJPG decoders); possible reasons for this:
    //         - parameter is an 'output' pin, and no input pin(s) is/are
    //           connected. This could mean that the filter only supports a
    //           specific set of transformations, dependant on the input type
    // *TODO*: find out exactly why this happens
    if (!SUCCEEDED (result))
      break;
  
    // sanity check(s)
    ACE_ASSERT (media_types_a[0]);

    if (!Stream_MediaFramework_Tools::isCompressed (media_types_a[0]->subtype,
                                                    deviceCategory_in,
                                                    STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
      break;

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_types_a[0]);
  } while (true);
  enumerator_p->Release ();

  mediaType_inout = media_types_a[0];

  return !!mediaType_inout;
}

unsigned int
Stream_MediaFramework_DirectShow_Tools::countFormats (IPin* pin_in,
                                                      REFGUID formatType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::countFormats"));

  unsigned int result = 0;

  // sanity check(s)
  ACE_ASSERT (pin_in);

  IEnumMediaTypes* enumerator_p = NULL;
  HRESULT result_2 = pin_in->EnumMediaTypes (&enumerator_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    return result;
  } // end IF

  struct _AMMediaType* media_types_a[1];
  ACE_OS::memset (media_types_a, 0, sizeof (media_types_a));
  ULONG fetched = 0;
  do
  {
    result_2 = enumerator_p->Next (1,
                                   media_types_a,
                                   &fetched);
    if (FAILED (result_2) ||
        (result_2 == S_FALSE)) // most probable reason: pin is not connected
      break;
    ACE_ASSERT (media_types_a[0]);
    if (!InlineIsEqualGUID (formatType_in, GUID_NULL) &&
        !InlineIsEqualGUID (formatType_in, media_types_a[0]->formattype))
      goto continue_;

    ++result;
#if defined (_DEBUG)
    Stream_MediaFramework_DirectShow_Tools::dump (*media_types_a[0]);
#endif

continue_:
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_types_a[0]);
  } while (true);
  enumerator_p->Release (); enumerator_p = NULL;

  return result;
}

struct _AMMediaType*
Stream_MediaFramework_DirectShow_Tools::copy (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::copy"));

  // initialize return value(s)
  struct _AMMediaType* result_p =
    static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
  if (!result_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    return NULL;
  } // end IF
  ACE_OS::memset (result_p, 0, sizeof (struct _AMMediaType));

  HRESULT result = CopyMediaType (result_p,
                                  &mediaType_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CopyMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    CoTaskMemFree (result_p); result_p = NULL;
    return NULL;
  } // end IF

  return result_p;
}

bool
Stream_MediaFramework_DirectShow_Tools::match (const struct tagBITMAPINFOHEADER& bitmapInfo_in,
                                               const struct tagBITMAPINFOHEADER& bitmapInfo2_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::match"));

  if (bitmapInfo_in.biBitCount != bitmapInfo2_in.biBitCount)
    return false;
  if (bitmapInfo_in.biClrImportant != bitmapInfo2_in.biClrImportant)
    return false;
  if (bitmapInfo_in.biClrUsed != bitmapInfo2_in.biClrUsed)
    return false;
  if (bitmapInfo_in.biCompression != bitmapInfo2_in.biCompression)
    return false;
  if ((bitmapInfo_in.biHeight != bitmapInfo2_in.biHeight) &&
      (bitmapInfo_in.biHeight != -bitmapInfo2_in.biHeight))
    return false;
  if (bitmapInfo_in.biPlanes != bitmapInfo2_in.biPlanes)
    return false;
  if (bitmapInfo_in.biSize != bitmapInfo2_in.biSize)
    return false;
  if (bitmapInfo_in.biSizeImage != bitmapInfo2_in.biSizeImage)
    return false;
  if (bitmapInfo_in.biWidth != bitmapInfo2_in.biWidth)
    return false;
  if (bitmapInfo_in.biXPelsPerMeter != bitmapInfo2_in.biXPelsPerMeter)
    return false;
  if (bitmapInfo_in.biYPelsPerMeter != bitmapInfo2_in.biYPelsPerMeter)
    return false;

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::match (const struct tagVIDEOINFOHEADER& videoInfo_in,
                                               const struct tagVIDEOINFOHEADER& videoInfo2_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::match"));

  if (videoInfo_in.AvgTimePerFrame != videoInfo2_in.AvgTimePerFrame)
    return false;
  if (videoInfo_in.dwBitErrorRate != videoInfo2_in.dwBitErrorRate)
    return false;
  if (videoInfo_in.dwBitRate != videoInfo2_in.dwBitRate)
    return false;
  if (!Stream_MediaFramework_DirectShow_Tools::match (videoInfo_in.bmiHeader,
                                                      videoInfo2_in.bmiHeader))
    return false;

  return true;
}
bool
Stream_MediaFramework_DirectShow_Tools::match (const struct tagVIDEOINFOHEADER2& videoInfo_in,
                                               const struct tagVIDEOINFOHEADER2& videoInfo2_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::match"));

  if (videoInfo_in.AvgTimePerFrame != videoInfo2_in.AvgTimePerFrame)
    return false;
  if (videoInfo_in.dwBitErrorRate != videoInfo2_in.dwBitErrorRate)
    return false;
  if (videoInfo_in.dwBitRate != videoInfo2_in.dwBitRate)
    return false;
  //if (videoInfo_in.dwControlFlags != videoInfo2_in.dwControlFlags)
  //  return false;
  //if (videoInfo_in.dwCopyProtectFlags != videoInfo2_in.dwCopyProtectFlags)
  //  return false;
  if (videoInfo_in.dwInterlaceFlags != videoInfo2_in.dwInterlaceFlags)
    return false;
  if (videoInfo_in.dwPictAspectRatioX != videoInfo2_in.dwPictAspectRatioX)
    return false;
  if (videoInfo_in.dwPictAspectRatioY != videoInfo2_in.dwPictAspectRatioY)
    return false;
  return Stream_MediaFramework_DirectShow_Tools::match (videoInfo_in.bmiHeader,
                                                        videoInfo2_in.bmiHeader);
}

void
Stream_MediaFramework_DirectShow_Tools::free (Stream_MediaFramework_DirectShow_Formats_t& mediaTypes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::free"));

  while (!mediaTypes_in.empty ())
  {
    FreeMediaType (mediaTypes_in.front ());
    mediaTypes_in.pop_front ();
  } // end WHILE
}

bool
Stream_MediaFramework_DirectShow_Tools::match (const struct _AMMediaType& mediaType_in,
                                               const struct _AMMediaType& mediaType2_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::match"));

  //CMediaType media_type (mediaType_in);
  //CMediaType media_type_2 (mediaType2_in);
  // step1: match all GUIDs
  //if (!media_type.MatchesPartial (&media_type_2))
  //{
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s does not match %s\n"),
  //              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (mediaType_in, true).c_str ()),
  //              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (mediaType2_in, true).c_str ())));
  //  return false;
  //} // end IF

  if (!InlineIsEqualGUID (mediaType2_in.majortype, GUID_NULL) &&
      !InlineIsEqualGUID (mediaType_in.majortype, mediaType2_in.majortype))
    return false;
  if (!InlineIsEqualGUID (mediaType2_in.subtype, GUID_NULL) &&
      !InlineIsEqualGUID (mediaType_in.subtype, mediaType2_in.subtype))
    return false;

  if (!InlineIsEqualGUID (mediaType2_in.formattype, GUID_NULL))
  {
    // if the format block is specified then it must match exactly
    if (!InlineIsEqualGUID (mediaType_in.formattype, mediaType2_in.formattype))
      return false;
    // *NOTE*: one exception to the rule is in video formats; if the video
    //         is merely flipped (i.e. has negative height), this is acceptable
    if (InlineIsEqualGUID (mediaType_in.majortype, MEDIATYPE_Video))
    {
      if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
      {
        struct tagVIDEOINFOHEADER* video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
        struct tagVIDEOINFOHEADER* video_info_header_2 =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType2_in.pbFormat);
        if (!Stream_MediaFramework_DirectShow_Tools::match (*video_info_header_p,
                                                            *video_info_header_2))
          return false;
      } // end IF
      else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
      {
        struct tagVIDEOINFOHEADER2* video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_in.pbFormat);
        struct tagVIDEOINFOHEADER2* video_info_header2_2 =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType2_in.pbFormat);
        if (!Stream_MediaFramework_DirectShow_Tools::match (*video_info_header2_p,
                                                            *video_info_header2_2))
          return false;
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), falling back\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
        goto fallback;
      }
    } // end IF

    goto continue_;

fallback:
    if (mediaType_in.cbFormat != mediaType2_in.cbFormat)
      return false;
    if (mediaType_in.cbFormat &&
        ACE_OS::memcmp (mediaType_in.pbFormat, mediaType2_in.pbFormat, mediaType_in.cbFormat))
      return false;
  } // end IF
continue_:

  return true;
}

void
Stream_MediaFramework_DirectShow_Tools::resize (const Common_Image_Resolution_t& size_in,
                                                struct _AMMediaType& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::resize"));

  if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_inout.pbFormat;

    video_info_header_p->bmiHeader.biWidth = size_in.cx;
    video_info_header_p->bmiHeader.biHeight = size_in.cy;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);

    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                      // bits / frame
      (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
  } // end IF
  else if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_inout.pbFormat;

    video_info_header2_p->bmiHeader.biWidth = size_in.cx;
    video_info_header2_p->bmiHeader.biHeight = size_in.cy;
    video_info_header2_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header2_p->bmiHeader);

    video_info_header2_p->dwBitRate =
      (video_info_header2_p->bmiHeader.biSizeImage * 8) *                      // bits / frame
      (10000000 / static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame)); // fps
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_inout.formattype).c_str ())));
  } // end ELSE
}

void
Stream_MediaFramework_DirectShow_Tools::setFormat (REFGUID mediaSubType_in,
                                                   struct _AMMediaType& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::setFormat"));

  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (mediaSubType_in, GUID_NULL));

  mediaType_inout.subtype = mediaSubType_in;

  FOURCCMap fourcc_map (&mediaType_inout.subtype);
  if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_inout.pbFormat;

    video_info_header_p->bmiHeader.biCompression =
      (Stream_MediaFramework_Tools::isCompressedVideo (mediaType_inout.subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ? fourcc_map.GetFOURCC ()
                                                                                         : BI_RGB);
  } // end IF
  else if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_inout.pbFormat;

    video_info_header2_p->bmiHeader.biCompression =
      (Stream_MediaFramework_Tools::isCompressedVideo (mediaType_inout.subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ? fourcc_map.GetFOURCC ()
                                                                                         : BI_RGB);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_inout.formattype).c_str ())));
  } // end ELSE
  // *TOOD*: update lSampleSize, dwBitRate, biBitCount, biCompression and
  //         biSizeImage accordingly
}

void
Stream_MediaFramework_DirectShow_Tools::setResolution (const Common_Image_Resolution_t& resolution_in,
                                                       struct _AMMediaType& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::setResolution"));

  unsigned int frames_per_second_i = 0, frame_size_i = 0;
  if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_inout.pbFormat;

    video_info_header_p->bmiHeader.biWidth = resolution_in.cx;
    video_info_header_p->bmiHeader.biHeight = resolution_in.cy;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    frames_per_second_i =
      10000000 / static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * frames_per_second_i) * 8;
    frame_size_i = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_inout.pbFormat;

    video_info_header2_p->bmiHeader.biWidth = resolution_in.cx;
    video_info_header2_p->bmiHeader.biHeight = resolution_in.cy;
    video_info_header2_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header2_p->bmiHeader);
    frames_per_second_i =
      10000000 / static_cast<unsigned int> (video_info_header2_p->AvgTimePerFrame);
    video_info_header2_p->dwBitRate =
      (video_info_header2_p->bmiHeader.biSizeImage * frames_per_second_i) * 8;
    frame_size_i = video_info_header2_p->bmiHeader.biSizeImage;
  } // end ELSE IFs
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_inout.formattype).c_str ())));
    return;
  } // end ELSE
  ACE_ASSERT (frame_size_i);
  mediaType_inout.lSampleSize = frame_size_i;
}

void
Stream_MediaFramework_DirectShow_Tools::setFramerate (const unsigned int& frameRate_in,
                                                      struct _AMMediaType& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::setFramerate"));

  if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_inout.pbFormat;
    video_info_header_p->AvgTimePerFrame = 10000000 / frameRate_in;
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * frameRate_in) * 8;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_inout.pbFormat;
    video_info_header2_p->AvgTimePerFrame = 10000000 / frameRate_in;
    video_info_header2_p->dwBitRate =
      (video_info_header2_p->bmiHeader.biSizeImage * frameRate_in) * 8;
  } // end ELSE IFs
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_inout.formattype).c_str ())));
    return;
  } // end ELSE
}

struct _AMMediaType
Stream_MediaFramework_DirectShow_Tools::toRGB (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toRGB"));

  // initialize return value(s)
  struct _AMMediaType result_s;
  ACE_OS::memset (&result_s, 0, sizeof (struct _AMMediaType));

  struct _AMMediaType* media_type_p =
    Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    return result_s;
  } // end IF
  result_s = *media_type_p;
  CoTaskMemFree (media_type_p); media_type_p = NULL;

  if (Stream_MediaFramework_Tools::isRGB (result_s.subtype,
                                          STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
    return result_s; // nothing to do

  HRESULT result_2 = E_FAIL;
  ACE_ASSERT (InlineIsEqualGUID (result_s.majortype, MEDIATYPE_Video));
  result_s.subtype =
    (Stream_MediaFramework_Tools::isRGB (STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT,
                                         STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ? STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT
                                                                           : MEDIASUBTYPE_RGB32);
  result_s.bFixedSizeSamples = TRUE;
  result_s.bTemporalCompression = FALSE;
  if (InlineIsEqualGUID (result_s.formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (result_s.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    struct tagVIDEOINFOHEADER* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (result_s.pbFormat);
    // *NOTE*: empty --> use entire video
    result_2 = SetRectEmpty (&video_info_header_p->rcSource);
    ACE_ASSERT (SUCCEEDED (result_2));
    result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
    // *NOTE*: empty --> fill entire buffer
    ACE_ASSERT (SUCCEEDED (result_2));
    //ACE_ASSERT (video_info_header_p->dwBitRate);
    ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
    //ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
    ACE_ASSERT (video_info_header_p->bmiHeader.biWidth);
    ACE_ASSERT (video_info_header_p->bmiHeader.biHeight);
    //if (video_info_header_p->bmiHeader.biHeight > 0)
    //  video_info_header_p->bmiHeader.biHeight =
    //    -video_info_header_p->bmiHeader.biHeight;
    //ACE_ASSERT (video_info_header_p->bmiHeader.biHeight < 0);
    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
    video_info_header_p->bmiHeader.biBitCount =
      Stream_MediaFramework_Tools::toBitCount (result_s.subtype);
    ACE_ASSERT (video_info_header_p->bmiHeader.biBitCount);
    video_info_header_p->bmiHeader.biCompression = BI_RGB;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    ////video_info_header_p->bmiHeader.biXPelsPerMeter;
    ////video_info_header_p->bmiHeader.biYPelsPerMeter;
    ////video_info_header_p->bmiHeader.biClrUsed;
    ////video_info_header_p->bmiHeader.biClrImportant;
    ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
      (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
    result_s.lSampleSize =
      video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else if (InlineIsEqualGUID (result_s.formattype, FORMAT_VideoInfo2))
  {
    ACE_ASSERT (result_s.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
    struct tagVIDEOINFOHEADER2* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (result_s.pbFormat);
    // *NOTE*: empty --> use entire video
    result_2 = SetRectEmpty (&video_info_header_p->rcSource);
    ACE_ASSERT (SUCCEEDED (result_2));
    result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
    // *NOTE*: empty --> fill entire buffer
    ACE_ASSERT (SUCCEEDED (result_2));
    //ACE_ASSERT (video_info_header_p->dwBitRate);
    ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
    //ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    ACE_ASSERT (video_info_header_p->dwInterlaceFlags == 0);
    ACE_ASSERT (video_info_header_p->dwCopyProtectFlags == 0);
    ACE_ASSERT (video_info_header_p->dwPictAspectRatioX);
    ACE_ASSERT (video_info_header_p->dwPictAspectRatioY);
    ACE_ASSERT (video_info_header_p->dwReserved1 == 0);
    ACE_ASSERT (video_info_header_p->dwReserved2 == 0);
    ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
    ACE_ASSERT (video_info_header_p->bmiHeader.biWidth);
    ACE_ASSERT (video_info_header_p->bmiHeader.biHeight);
    //if (video_info_header_p->bmiHeader.biHeight > 0)
    //  video_info_header_p->bmiHeader.biHeight =
    //    -video_info_header_p->bmiHeader.biHeight;
    //ACE_ASSERT (video_info_header_p->bmiHeader.biHeight < 0);
    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
    video_info_header_p->bmiHeader.biBitCount =
      Stream_MediaFramework_Tools::toBitCount (result_s.subtype);
    ACE_ASSERT (video_info_header_p->bmiHeader.biBitCount);
    video_info_header_p->bmiHeader.biCompression = BI_RGB;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    ////video_info_header_p->bmiHeader.biXPelsPerMeter;
    ////video_info_header_p->bmiHeader.biYPelsPerMeter;
    ////video_info_header_p->bmiHeader.biClrUsed;
    ////video_info_header_p->bmiHeader.biClrImportant;
    ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
      (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
    result_s.lSampleSize =
      video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (result_s.formattype).c_str ())));
    Stream_MediaFramework_DirectShow_Tools::free (result_s);
  } // end ELSE

  return result_s;
}

std::string
Stream_MediaFramework_DirectShow_Tools::toString (const struct _AMMediaType& mediaType_in,
                                                  bool condensed_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toString"));

  if (condensed_in)
    return Stream_MediaFramework_DirectShow_Tools::toString_2 (mediaType_in);

  std::string result;

  Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator =
    Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.find (mediaType_in.majortype);
  result = ACE_TEXT_ALWAYS_CHAR ("majortype: \"");
  if (iterator == Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap.end ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media majortype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.majortype).c_str ())));
    result += Common_Tools::GUIDToString (mediaType_in.majortype);
  } // end IF
  else
    result += (*iterator).second;
  result += ACE_TEXT_ALWAYS_CHAR ("\"\nsubtype: \"");
  result +=
    Stream_MediaFramework_Tools::mediaSubTypeToString (mediaType_in.subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

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
    Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.find (mediaType_in.formattype);
  if (iterator == Stream_MediaFramework_Tools::Stream_MediaFramework_FormatTypeToStringMap.end ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.formattype).c_str ())));
    result += Common_Tools::GUIDToString (mediaType_in.formattype);
  } // end IF
  else
    result += (*iterator).second;

  result += ACE_TEXT_ALWAYS_CHAR ("\"\npUnk: 0x");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << std::showbase << std::hex
            << mediaType_in.pUnk
            << std::dec;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\ncbFormat: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.cbFormat;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\npbFormat: 0x");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << std::showbase << std::hex
            << static_cast<void*> (mediaType_in.pbFormat)
            << std::dec;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
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
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
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
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
    result += ACE_TEXT_ALWAYS_CHAR ("---\nwFormatTag: \"");
    WORD_TO_STRING_MAP_ITERATOR_T iterator =
      Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.find (waveformatex_p->wFormatTag);
    if (iterator == Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatTypeToStringMap.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown wave formattype (was: %d), aborting\n"),
                  waveformatex_p->wFormatTag));
      return std::string ();
    } // end IF
    result += (*iterator).second;

    result += ACE_TEXT_ALWAYS_CHAR ("\"\nnChannels: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nChannels;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nnSamplesPerSec: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nSamplesPerSec;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nnAvgBytesPerSec: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nAvgBytesPerSec;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nnBlockAlign: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nBlockAlign;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nwBitsPerSample: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->wBitsPerSample;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ncbSize: ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->cbSize;
    result += converter.str ();

    if (waveformatex_p->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
      WAVEFORMATEXTENSIBLE* waveformatextensible_p =
        (WAVEFORMATEXTENSIBLE*)mediaType_in.pbFormat;

      // *TODO*: the second argument may not be entirely accurate
      if (Stream_MediaFramework_Tools::isCompressedAudio (mediaType_in.subtype,
                                                          STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
      {
        result += ACE_TEXT_ALWAYS_CHAR ("\nwSamplesPerBlock: ");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << waveformatextensible_p->Samples.wSamplesPerBlock;
        result += converter.str ();
      } // end IF
      else
      {
        result += ACE_TEXT_ALWAYS_CHAR ("\nwValidBitsPerSample: ");
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << waveformatextensible_p->Samples.wValidBitsPerSample;
        result += converter.str ();
      } // end ELSE

      result += ACE_TEXT_ALWAYS_CHAR ("\ndwChannelMask: 0x");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter <<
        std::hex << waveformatextensible_p->dwChannelMask << std::dec;
      result += converter.str ();

      result += ACE_TEXT_ALWAYS_CHAR ("\nSubFormat: \"");
      Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator =
        Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.find (waveformatextensible_p->SubFormat);
      if (iterator == Stream_MediaFramework_DirectShow_Tools::Stream_WaveFormatSubTypeToStringMap.end ())
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("invalid/unknown wave subformat (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
        result += Common_Tools::GUIDToString (waveformatextensible_p->SubFormat);
      } // end IF
      else
        result += (*iterator).second;
      result += ACE_TEXT_ALWAYS_CHAR ("\"\n");
    } // end IF
    else
      result += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end ELSE IF
  else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.formattype).c_str ())));

  return result;
}

Common_Image_Resolution_t
Stream_MediaFramework_DirectShow_Tools::toResolution (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toResolution"));

  Common_Image_Resolution_t result;
  ACE_OS::memset (&result, 0, sizeof (Common_Image_Resolution_t));

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result.cx = video_info_header_p->bmiHeader.biWidth;
    result.cy = ::abs (video_info_header_p->bmiHeader.biHeight);
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result.cx = video_info_header2_p->bmiHeader.biWidth;
    result.cy = ::abs (video_info_header2_p->bmiHeader.biHeight);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return result;
  } // end ELSE

  return result;
}

unsigned int
Stream_MediaFramework_DirectShow_Tools::toFramerate (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toFramerate"));

  unsigned int result = 0;

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result =
      (NANOSECONDS / static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame));
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result =
      (NANOSECONDS / static_cast<unsigned int> (video_info_header2_p->AvgTimePerFrame));
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return result;
  } // end ELSE

  return result;
}

unsigned int
Stream_MediaFramework_DirectShow_Tools::toFramesize (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toFramesize"));

  unsigned int result = 0;

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result = DIBSIZE (video_info_header_p->bmiHeader);
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result = DIBSIZE (video_info_header2_p->bmiHeader);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return result;
  } // end ELSE

  return result;
}

unsigned int
Stream_MediaFramework_DirectShow_Tools::toBitrate (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toBitrate"));

  unsigned int result = 0;

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result = video_info_header_p->dwBitRate;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result = video_info_header2_p->dwBitRate;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return result;
  } // end ELSE

  return result;
}

struct _AMMediaType*
Stream_MediaFramework_DirectShow_Tools::to (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::to"));

  // initialize return value(s)
  struct _AMMediaType dummy_s;
  ACE_OS::memset (&dummy_s, 0, sizeof (struct _AMMediaType));
  struct _AMMediaType* result_p = CreateMediaType (&dummy_s);
  ACE_ASSERT (result_p);

  BOOL result_2 = FALSE;
  result_p->majortype = MEDIATYPE_Video;
  result_p->subtype =
    Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType (mediaType_in.format);
  result_p->bFixedSizeSamples = TRUE;
  result_p->bTemporalCompression = FALSE;
  result_p->formattype = FORMAT_VideoInfo;
  result_p->cbFormat = sizeof (struct tagVIDEOINFOHEADER);
  result_p->pbFormat =
    reinterpret_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tagVIDEOINFOHEADER)));
  ACE_ASSERT (result_p->pbFormat);
  ACE_OS::memset (result_p->pbFormat, 0, sizeof (struct tagVIDEOINFOHEADER));
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (result_p->pbFormat);
  // *NOTE*: empty --> use entire video
  result_2 = SetRectEmpty (&video_info_header_p->rcSource);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
  // *NOTE*: empty --> fill entire buffer
  ACE_ASSERT (SUCCEEDED (result_2));
  //video_info_header_p->dwBitRate = ;
  video_info_header_p->dwBitErrorRate = 0;
  //video_info_header_p->AvgTimePerFrame = ;
  video_info_header_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_header_p->bmiHeader.biWidth = mediaType_in.resolution.cx;
  video_info_header_p->bmiHeader.biHeight = mediaType_in.resolution.cy;
  //if (video_info_header_p->bmiHeader.biHeight > 0)
  //  video_info_header_p->bmiHeader.biHeight =
  //    -video_info_header_p->bmiHeader.biHeight;
  //ACE_ASSERT (video_info_header_p->bmiHeader.biHeight < 0);
  video_info_header_p->bmiHeader.biPlanes = 1;
  video_info_header_p->bmiHeader.biBitCount =
    Stream_MediaFramework_Tools::toBitCount (result_p->subtype);
  //ACE_ASSERT (video_info_header_p->bmiHeader.biBitCount);
  video_info_header_p->bmiHeader.biCompression = BI_RGB;
  video_info_header_p->bmiHeader.biSizeImage =
    DIBSIZE (video_info_header_p->bmiHeader);
  ////video_info_header_p->bmiHeader.biXPelsPerMeter;
  ////video_info_header_p->bmiHeader.biYPelsPerMeter;
  ////video_info_header_p->bmiHeader.biClrUsed;
  ////video_info_header_p->bmiHeader.biClrImportant;
  ACE_ASSERT (mediaType_in.frameRate.den);
  video_info_header_p->AvgTimePerFrame =
    ((mediaType_in.frameRate.num * 100000000000) / mediaType_in.frameRate.den) / NANOSECONDS;
  video_info_header_p->dwBitRate =
    (video_info_header_p->AvgTimePerFrame ? (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
                                            (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)) // fps
                                          : 0);
  result_p->lSampleSize = video_info_header_p->bmiHeader.biSizeImage;

  return result_p;
}
