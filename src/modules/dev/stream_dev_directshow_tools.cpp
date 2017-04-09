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

#include "stream_dev_directshow_tools.h"

#include <sstream>

#include <ace/Log_Msg.h>

#include <oleauto.h>

#include <initguid.h> // *NOTE*: this exports DEFINE_GUIDs (see e.g. dxva.h)
#include <dmoreg.h>
#include <dshow.h>
#include <dsound.h>
#include <dvdmedia.h>
#include <dxva.h>
#include <Dmodshow.h>
#include <evr.h>
#include <fourcc.h>
#include <ks.h>
#include <ksmedia.h>
 //#include <ksuuids.h>
#include <qedit.h>

#include <mfapi.h>
#include <mferror.h>
//#include <mftransform.h>

#include <wmcodecdsp.h>

#include "common_time_common.h"
#include "common_tools.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

// initialize statics
Stream_Module_Device_DirectShow_Tools::GUID2STRING_MAP_T Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap;
Stream_Module_Device_DirectShow_Tools::WORD2STRING_MAP_T Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap;
Stream_Module_Device_DirectShow_Tools::GUID2STRING_MAP_T Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap;
ACE_HANDLE Stream_Module_Device_DirectShow_Tools::logFileHandle = ACE_INVALID_HANDLE;

void
Stream_Module_Device_DirectShow_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::initialize"));

  // DirectShow
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Video, ACE_TEXT_ALWAYS_CHAR ("vids")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Audio, ACE_TEXT_ALWAYS_CHAR ("auds")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Text, ACE_TEXT_ALWAYS_CHAR ("txts")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Midi, ACE_TEXT_ALWAYS_CHAR ("mids")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Stream, ACE_TEXT_ALWAYS_CHAR ("Stream")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Interleaved, ACE_TEXT_ALWAYS_CHAR ("iavs")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_File, ACE_TEXT_ALWAYS_CHAR ("file")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_ScriptCommand, ACE_TEXT_ALWAYS_CHAR ("scmd")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_AUXLine21Data, ACE_TEXT_ALWAYS_CHAR ("AUXLine21Data")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_AUXTeletextPage, ACE_TEXT_ALWAYS_CHAR ("AUXTeletextPage")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_CC_CONTAINER, ACE_TEXT_ALWAYS_CHAR ("CC_CONTAINER")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_DTVCCData, ACE_TEXT_ALWAYS_CHAR ("DTVCCData")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MSTVCaption, ACE_TEXT_ALWAYS_CHAR ("MSTVCaption")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_VBI, ACE_TEXT_ALWAYS_CHAR ("VBI")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_Timecode, ACE_TEXT_ALWAYS_CHAR ("Timecode")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_LMRT, ACE_TEXT_ALWAYS_CHAR ("lmrt")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_URL_STREAM, ACE_TEXT_ALWAYS_CHAR ("URL_STREAM")));

  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PACK, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PACK")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PES, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PES")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_SECTIONS, ACE_TEXT_ALWAYS_CHAR ("MPEG2_SECTIONS")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_MPEG2_PACK, ACE_TEXT_ALWAYS_CHAR ("MPEG2_PACK")));

  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_DVD_ENCRYPTED_PACK, ACE_TEXT_ALWAYS_CHAR ("DVD_ENCRYPTED_PACK")));
  Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MEDIATYPE_DVD_NAVIGATION, ACE_TEXT_ALWAYS_CHAR ("DVD_NAVIGATION")));

  // ---------------------------------------------------------------------------

  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_UNKNOWN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VSELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_IBM_CVSD, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ALAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MULAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DTS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DRM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WMAVOICE9, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WMAVOICE10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OKI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DVI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_IMA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MEDIASPACE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SIERRA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_G723_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIGISTD, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIGIFIX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIALOGIC_OKI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MEDIAVISION_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CU_CODEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_HP_DYN_VOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_YAMAHA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SONARC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DSPGROUP_TRUESPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ECHOSC1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_AUDIOFILE_AF36, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_APTX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_AUDIOFILE_AF10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PROSODY_1612, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LRC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DOLBY_AC2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MSNAUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ANTEX_ADPCME, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CONTROL_RES_VQLPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIGIREAL, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIGIADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CONTROL_RES_CR10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NMS_VBXADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CS_IMAADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ECHOSC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ROCKWELL_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ROCKWELL_DIGITALK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_XEBEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_G721_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_G728_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MSG723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SHARP_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MPEG, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MPEGLAYER3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CIRRUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ESPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CANOPUS_ATRAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_G726_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_G722_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DSAT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DSAT_DISPLAY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_BYTE_ALIGNED, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC16, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC20, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT29, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT29HW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_VR12, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_VR18, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_TQ40, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_SC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_SC3_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SOFTSOUND, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_TQ60, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MSRT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_G729A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MVI_MVI2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DF_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DF_GSM610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ISIAUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ONLIVE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MULTITUDE_FT_SX20, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_INFOCOM_ITS_G721_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CONVEDIA_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CONGRUENCY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SBC24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MEDIASONIC_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PROSODY_8KBPS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ZYXEL_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_LPCBB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PACKED, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MALDEN_PHONYTALK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_GSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_G720_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_TETRA_ACELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NEC_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RAW_AAC1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RHETOREX_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_IRAT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VIVO_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VIVO_SIREN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_GRUNDIG, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIGITAL_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SANYO_LD_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACEPLNET, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACELP4800, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACELP8V3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_G729A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_KELVIN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOICEAGE_AMR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_G726ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DICTAPHONE_CELP68, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DICTAPHONE_CELP54, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_QUALCOMM_PUREVOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_QUALCOMM_HALFRATE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_TUBGSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MSAUDIO1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WMASPDIF, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ULAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ALAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_16K, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC008, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_G726L, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_CELP54, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_CELP68, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_KNOWLEDGE_ADVENTURE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_FRAUNHOFER_IIS_MPEG2_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DTS_DS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_FASTSPEECH8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_FASTSPEECH10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_UHER_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ULEAD_DV_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ULEAD_DV_AUDIO_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_QUARTERDECK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ILINK_VC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RAW_SPORT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ESST_AC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GENERIC_PASSTHRU, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_IPI_HSX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_IPI_RPELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CS2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SCX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SCY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SONY_ATRAC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_TELUM_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_TELUM_IA_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NORCOM_VOICE_SYSTEMS_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_FM_TOWNS_SND, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MICRONAS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MICRONAS_CELP833, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_BTV_DIGITAL, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_MUSIC_CODER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_INDEO_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_QDESIGN_MUSIC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ON2_VP7_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ON2_VP6_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VME_VMPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_TPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LIGHTWAVE_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OLIGSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OLIADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OLICELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OLISBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OLIOPR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC12, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC16, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NORRIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ISIAUDIO_2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_LOAS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VODAFONE_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VODAFONE_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT24_SPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SONICFOUNDRY_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_INNINGS_TELECOM_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_SX8300P, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_SX5363S, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CUSEEME, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NTCSOFT_ALF2CM_ACM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DVM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DTS2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MAKEAVIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIVIO_MPEG4_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_ADAPTIVE_MULTIRATE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DIVIO_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LEAD_SPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_LEAD_VORBIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WAVPACK_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_1_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_2_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_3_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_3COM_NBX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_FAAD_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_AMR_NB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_AMR_WP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM_AMR_CBR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM_AMR_VBR_SID, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_AVQSBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_SBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SYMBOL_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOICEAGE_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_INGENIENT_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_MPEG4_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ENCORE_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_ZOLL_ASAO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_SPEEX_VOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VIANIX_MASC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WM9_SPECTRUM_ANALYZER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_WMF_SPECTRUM_ANAYZER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM_610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM_620, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM_660, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM_690, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GSM_ADAPTIVE_MULTIRATE_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G722, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_SIREN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_GLOBAL_IP_ILBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_RADIOTIME_TIME_SHIFT_RADIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NICE_ACA, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NICE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G721, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G722_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_LBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_NICE_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_FRACE_TELECOM_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_CODIAN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_FLAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_EXTENSIBLE, ACE_TEXT_ALWAYS_CHAR ("EXTENSIBLE")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.insert (std::make_pair (WAVE_FORMAT_DEVELOPMENT, ACE_TEXT_ALWAYS_CHAR ("DEVELOPMENT")));

  // ---------------------------------------------------------------------------

  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ANALOG, ACE_TEXT_ALWAYS_CHAR ("Analog")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_DRM, ACE_TEXT_ALWAYS_CHAR ("DRM")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ALAW, ACE_TEXT_ALWAYS_CHAR ("ALAW")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MULAW, ACE_TEXT_ALWAYS_CHAR ("MULAW")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("ADPCM")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEG, ACE_TEXT_ALWAYS_CHAR ("MPEG")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL, ACE_TEXT_ALWAYS_CHAR ("DOLBY_DIGITAL")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_WMA_PRO, ACE_TEXT_ALWAYS_CHAR ("WMA_PRO")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DTS, ACE_TEXT_ALWAYS_CHAR ("DTS")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG1, ACE_TEXT_ALWAYS_CHAR ("MPEG1")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG2, ACE_TEXT_ALWAYS_CHAR ("MPEG2")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG3, ACE_TEXT_ALWAYS_CHAR ("MPEG3")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_AAC, ACE_TEXT_ALWAYS_CHAR ("AAC")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_ATRAC, ACE_TEXT_ALWAYS_CHAR ("ATRAC")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_ONE_BIT_AUDIO, ACE_TEXT_ALWAYS_CHAR ("ONE_BIT_AUDIO")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS, ACE_TEXT_ALWAYS_CHAR ("DOLBY_DIGITAL_PLUS")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD, ACE_TEXT_ALWAYS_CHAR ("DTS_HD")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP, ACE_TEXT_ALWAYS_CHAR ("DOLBY_MLP")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DST, ACE_TEXT_ALWAYS_CHAR ("DST")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEGLAYER3, ACE_TEXT_ALWAYS_CHAR ("MPEGLAYER3")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR ("HEAAC")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO2, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO2")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO3, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO3")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO_LOSSLESS")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_DTS_AUDIO, ACE_TEXT_ALWAYS_CHAR ("DTS_AUDIO")));
  Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_SDDS_AUDIO, ACE_TEXT_ALWAYS_CHAR ("SDDS_AUDIO")));
}

bool
Stream_Module_Device_DirectShow_Tools::addToROT (IFilterGraph* filterGraph_in,
                                                 DWORD& ID_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::addToROT"));

  // initialize return value(s)
  ID_out = 0;

  // sanity check(s)
  ACE_ASSERT (filterGraph_in);

  IUnknown* iunknown_p = filterGraph_in;
  IRunningObjectTable* ROT_p = NULL;
  IMoniker* moniker_p = NULL;
  WCHAR buffer[BUFSIZ];

  HRESULT result = GetRunningObjectTable (0, &ROT_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to GetRunningObjectTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (ROT_p);

  // *IMPORTANT NOTE*: do not change this syntax, otherwise graphedt.exe
  //                   cannot find the graph
  result =
    ::StringCchPrintfW (buffer, NUMELMS (buffer),
                        ACE_TEXT_ALWAYS_WCHAR ("FilterGraph %08x pid %08x"),
                        (DWORD_PTR)iunknown_p, ACE_OS::getpid ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to StringCchPrintfW(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result = CreateItemMoniker (ACE_TEXT_ALWAYS_WCHAR ("!"), buffer,
                              &moniker_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateItemMoniker(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
Stream_Module_Device_DirectShow_Tools::removeFromROT (DWORD id_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::removeFromROT"));

  // sanity check(s)
  ACE_ASSERT (id_in);

  IRunningObjectTable* ROT_p = NULL;
  HRESULT result = GetRunningObjectTable (0, &ROT_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to GetRunningObjectTable(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (ROT_p);

  result = ROT_p->Revoke (id_in);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d): \"%s\", continuing\n"),
                id_in,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed filter graph from running object table (ID was: %d)\n"),
                id_in));

  ROT_p->Release ();

  return true;
} // end IF

void
Stream_Module_Device_DirectShow_Tools::debug (IGraphBuilder* builder_in,
                                              const std::string& fileName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::debug"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (Stream_Module_Device_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE)
    goto continue_;

  if (!fileName_in.empty ())
  {
    Stream_Module_Device_DirectShow_Tools::logFileHandle =
      ACE_TEXT_CreateFile (ACE_TEXT (fileName_in.c_str ()),
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           CREATE_ALWAYS, // TRUNCATE_EXISTING :-)
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (Stream_Module_Device_DirectShow_Tools::logFileHandle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CreateFile(\"%s\"): \"%s\", returning\n"),
                  ACE_TEXT (fileName_in.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
      return;
    } // end IF
  } // end IF

continue_:
  HRESULT result =
    builder_in->SetLogFile (((Stream_Module_Device_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE) ? reinterpret_cast<DWORD_PTR> (Stream_Module_Device_DirectShow_Tools::logFileHandle)
                                                                                                          : NULL));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::SetLogFile(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT (fileName_in.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    if (!CloseHandle (Stream_Module_Device_DirectShow_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));

    return;
  } // end IF

  if (fileName_in.empty () &&
      (Stream_Module_Device_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE))
  {
    if (!CloseHandle (Stream_Module_Device_DirectShow_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
  } // end IF
}

void
Stream_Module_Device_DirectShow_Tools::dump (const Stream_Module_Device_DirectShow_Graph_t& graphConfiguration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::dump"));

  std::string graph_layout_string = ACE_TEXT_ALWAYS_CHAR ("[");
  Stream_Module_Device_DirectShow_GraphConstIterator_t iterator = graphConfiguration_in.begin ();

  graph_layout_string +=
    Stream_Module_Decoder_Tools::mediaSubTypeToString ((*iterator).mediaType->subtype, false);
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
      ((*iterator).mediaType ? Stream_Module_Decoder_Tools::mediaSubTypeToString ((*iterator).mediaType->subtype, false)
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
Stream_Module_Device_DirectShow_Tools::dump (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::dump"));

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
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*media_types_a[0]).c_str ())));

    Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_types_a[0]);
    ++index;
  } // end WHILE

  ienum_media_types_p->Release ();
}
void
Stream_Module_Device_DirectShow_Tools::dump (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::dump"));

  LONG width = -1;
  LONG height = -1;

  // --> audio
  if (mediaType_in.formattype == FORMAT_WaveFormatEx)
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
    ACE_ASSERT (waveformatex_p);

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\" [rate/resolution/channels]: %d,%d,%d\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaType_in.subtype).c_str ()),
                waveformatex_p->nSamplesPerSec,
                waveformatex_p->wBitsPerSample,
                waveformatex_p->nChannels));
    
    return;
  } // end IF
  // --> video
  else if (mediaType_in.formattype == FORMAT_VideoInfo)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    ACE_ASSERT (video_info_header_p);

    width = video_info_header_p->bmiHeader.biWidth;
    height = video_info_header_p->bmiHeader.biHeight;
  } // end IF
  else if (mediaType_in.formattype == FORMAT_VideoInfo2)
  {
    struct tagVIDEOINFOHEADER2* video_info_header_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    ACE_ASSERT (video_info_header_p);

    width = video_info_header_p->bmiHeader.biWidth;
    height = video_info_header_p->bmiHeader.biHeight;
  } // end ELSE
  else if (mediaType_in.formattype != GUID_NULL) // <-- 'don't care'
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
    return;
  } // end ELSE

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\" - \"%s\": %dx%d\n"),
              ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaType_in.subtype).c_str ()),
              ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ()),
              width, height));
}

std::string
Stream_Module_Device_DirectShow_Tools::name (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::name"));

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
Stream_Module_Device_DirectShow_Tools::pin (IBaseFilter* filter_in,
                                            enum _PinDirection direction_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::pin"));

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
    //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_in).c_str ()),
    //            ((direction_in == PINDIR_INPUT) ? ACE_TEXT ("input") : ACE_TEXT ("output"))));
    return NULL;
  } // end IF

  return result;
}

IBaseFilter*
Stream_Module_Device_DirectShow_Tools::pin2Filter (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::pin2Filter"));

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
Stream_Module_Device_DirectShow_Tools::name (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::name"));

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
Stream_Module_Device_DirectShow_Tools::loadDeviceGraph (const std::string& deviceName_in,
                                             REFGUID deviceCategory_in,
                                             IGraphBuilder*& IGraphBuilder_inout,
                                             IAMBufferNegotiation*& IAMBufferNegotiation_out,
                                             IAMStreamConfig*& IAMStreamConfig_out,
                                             Stream_Module_Device_DirectShow_Graph_t& graph_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::loadDeviceGraph"));

  // initialize return value(s)
  for (Stream_Module_Device_DirectShow_GraphIterator_t iterator = graph_out.begin ();
       iterator != graph_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_Module_Device_DirectShow_Tools::deleteMediaType ((*iterator).mediaType);
  graph_out.clear ();

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

  struct Stream_Module_Device_DirectShow_GraphEntry graph_entry;
  ICreateDevEnum* enumerator_p = NULL;
  IEnumMoniker* enum_moniker_p = NULL;
  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  VARIANT variant;
  IEnumPins* enumerator_2 = NULL;
  IPin* pin_p, *pin_2 = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
  DWORD returned_size = 0;

  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;
  if (!IGraphBuilder_inout)
  {
    ICaptureGraphBuilder2* builder_2 = NULL;
    result =
      CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&builder_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_CaptureGraphBuilder2): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
    if (!Stream_Module_Device_DirectShow_Tools::clear (IGraphBuilder_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (IGraphBuilder_inout);

  result =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (enumerator_p);

  result =
    enumerator_p->CreateClassEnumerator (deviceCategory_in,
                                         &enum_moniker_p,
                                         0);
  if (result != S_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    enumerator_p->Release ();

    //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    goto error;
  } // end IF
  ACE_ASSERT (enum_moniker_p);
  enumerator_p->Release ();

  while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
  {
    ACE_ASSERT (moniker_p);

    properties_p = NULL;
    result = moniker_p->BindToStorage (NULL, NULL,
                                       IID_PPV_ARGS (&properties_p));
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
    result =
      properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING,
                          &variant,
                          0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING),
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

  result = moniker_p->BindToObject (NULL, NULL,
                                    IID_PPV_ARGS (&filter_p));
  moniker_p->Release ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMoniker::BindToObject(IID_IBaseFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  if (deviceCategory_in == CLSID_AudioInputDeviceCategory)
    graph_entry.filterName =
      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (deviceCategory_in == CLSID_VideoInputDeviceCategory)
    graph_entry.filterName =
      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (deviceCategory_in).c_str ())));
    goto error;
  } // end ELSE
  result =
    IGraphBuilder_inout->AddFilter (filter_p,
                                    graph_entry.filterName.c_str ());
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
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
  graph_out.push_back (graph_entry);

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
    switch (pin_direction)
    {
      case PINDIR_INPUT:
      {
        if (deviceCategory_in == CLSID_AudioInputDeviceCategory)
        {
          IAMAudioInputMixer* audio_input_mixer_p = NULL;
          result = pin_p->QueryInterface (IID_PPV_ARGS (&audio_input_mixer_p));
          if (FAILED (result))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMAudioInputMixer): \"%s\", aborting\n"),
                        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
            goto error;
          } // end IF
          ACE_ASSERT (audio_input_mixer_p);

          struct _PinInfo pin_info_s;
          ACE_OS::memset (&pin_info_s, 0, sizeof (struct _PinInfo));
          result = pin_p->QueryPinInfo (&pin_info_s);
          ACE_ASSERT (SUCCEEDED (result));

          result = audio_input_mixer_p->put_Enable (TRUE);
          if (FAILED (result))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IAMAudioInputMixer::put_Enable(): \"%s\", aborting\n"),
                        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
            goto error;
          } // end IF
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("enabled input pin \"%s\"...\n"),
                      ACE_TEXT (ACE_TEXT_WCHAR_TO_TCHAR (pin_info_s.achName))));

          audio_input_mixer_p->Release ();
        } // end IF
        //else if (deviceCategory_in == CLSID_VideoInputDeviceCategory)

        pin_p->Release ();
        pin_p = NULL;

        continue;
      }
      case PINDIR_OUTPUT:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid pin direction (was: %d), aborting\n"),
                    pin_direction));

        // clean up
        pin_p->Release ();
        enumerator_p->Release ();

        goto error;
      }
    } // end SWITCH
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
    property_set_p->Release ();
    if (GUID_s == PIN_CATEGORY_CAPTURE)
      pin_2 = pin_p;
    else
      pin_p->Release ();
    pin_p = NULL;
  } // end WHILE
  enumerator_2->Release ();
  if (!pin_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("0x%@: no capture pin found, aborting\n"),
                filter_p));
    goto error;
  } // end IF

  result = pin_2->QueryInterface (IID_PPV_ARGS (&IAMBufferNegotiation_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMBufferNegotiation): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_2->Release ();

    goto error;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);

  result = pin_2->QueryInterface (IID_PPV_ARGS (&IAMStreamConfig_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_2->Release ();

    goto error;
  } // end IF
  ACE_ASSERT (IAMStreamConfig_out);
  pin_2->Release ();

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
Stream_Module_Device_DirectShow_Tools::loadSourceGraph (IBaseFilter* sourceFilter_in,
                                                        const std::wstring& sourceFilterName_in,
                                                        IGraphBuilder*& IGraphBuilder_inout,
                                                        IAMBufferNegotiation*& IAMBufferNegotiation_out,
                                                        IAMStreamConfig*& IAMStreamConfig_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::loadSourceGraph"));

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
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
    if (!Stream_Module_Device_DirectShow_Tools::clear (IGraphBuilder_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::clear(), aborting\n")));
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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ())));

  IEnumPins* enumerator_p = NULL;
  IPin* pin_p, *pin_2 = NULL;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
  DWORD returned_size = 0;

  result = sourceFilter_in->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (enumerator_p);

  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
  {
    ACE_ASSERT (pin_p);

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
    result =
      property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
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
    property_set_p->Release ();

    if (GUID_s == PIN_CATEGORY_CAPTURE)
    {
      pin_2 = pin_p;
      break;
    } // end IF

    pin_p->Release ();
    pin_p = NULL;
  } // end WHILE
  enumerator_p->Release ();
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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  result = pin_2->QueryInterface (IID_PPV_ARGS (&IAMStreamConfig_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_2->Release ();

    goto error;
  } // end IF
  ACE_ASSERT (IAMStreamConfig_out);
  pin_2->Release ();

  return true;

error:
  if (release_builder &&
      IGraphBuilder_inout)
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
Stream_Module_Device_DirectShow_Tools::loadAudioRendererGraph (const struct _AMMediaType& mediaType_in,
                                                               const int audioOutput_in,
                                                               IGraphBuilder* IGraphBuilder_in,
                                                               const CLSID& effect_in,
                                                               const Stream_Decoder_DirectShow_AudioEffectOptions& effectOptions_in,
                                                               Stream_Module_Device_DirectShow_Graph_t& graph_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::loadAudioRendererGraph"));

  HRESULT result = E_FAIL;
  struct Stream_Module_Device_DirectShow_GraphEntry graph_entry;
  struct _GUID GUID_s = GUID_NULL;

  // initialize return value(s)
  for (Stream_Module_Device_DirectShow_GraphIterator_t iterator = graph_out.begin ();
       iterator != graph_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_Module_Device_DirectShow_Tools::deleteMediaType ((*iterator).mediaType);
  graph_out.clear ();

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  //if (!IGraphBuilder_out)
  //{
  //  result =
  //    CoCreateInstance (CLSID_FilterGraph, NULL,
  //                      CLSCTX_INPROC_SERVER,
  //                      IID_PPV_ARGS (&IGraphBuilder_out));
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
    if (!Stream_Module_Device_DirectShow_Tools::resetGraph (IGraphBuilder_in,
                                                 CLSID_AudioInputDeviceCategory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::resetGraph(), aborting\n")));
      return false;
    } // end IF
  //} // end ELSE
  //ACE_ASSERT (IGraphBuilder_out);

  //// encode PCM --> WAV ?
  //struct _GUID converter_CLSID = WAV_Colour;
  //std::wstring converter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_PCM;
  //if (mediaType_in.subtype == MEDIASUBTYPE_WAVE)
  //{
  //  converter_CLSID = CLSID_MjpegDec;
  //  converter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG;
  //} // end IF
  //else if (mediaType_in.subtype == MEDIASUBTYPE_PCM)
  //{
  //  // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
  //  converter_CLSID = CLSID_AVIDec;
  //  converter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
  //} // end IF
  //else
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
  //              ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaType_in.subtype).c_str ())));
  //  return false;
  //} // end IF

  IBaseFilter* filter_p = NULL;
  //result = CoCreateInstance (converter_CLSID, NULL,
  //                           CLSCTX_INPROC_SERVER,
  //                           IID_PPV_ARGS (&filter_p));
  //if (FAILED (result))
  //{
  //  int result_2 = StringFromGUID2 (converter_CLSID,
  //                                  GUID_string, CHARS_IN_GUID);
  //  ACE_ASSERT (result_2 == CHARS_IN_GUID);
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
  //              ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return false;
  //} // end IF
  //ACE_ASSERT (filter_p);

  IBaseFilter* filter_2 = NULL;
  IBaseFilter* filter_3 = NULL;
  IBaseFilter* filter_4 = NULL;

  //result = IGraphBuilder_in->AddFilter (filter_p,
  //                                      converter_name.c_str ());
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added \"%s\"...\n"),
  //            ACE_TEXT_WCHAR_TO_TCHAR (converter_name.c_str ())));

  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_2));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_SampleGrabber).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_2);
  result = IGraphBuilder_in->AddFilter (filter_2,
                                        MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));

  // add effect DMO ?
  if (effect_in == GUID_NULL)
    goto continue_;

  result = CoCreateInstance (CLSID_DMOWrapperFilter, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_3));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_DMOWrapperFilter).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_3);
  IDMOWrapperFilter* wrapper_filter_p = NULL;
  result = filter_3->QueryInterface (IID_PPV_ARGS (&wrapper_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IDMOWrapperFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = wrapper_filter_p->Init (effect_in,
                                   DMOCATEGORY_AUDIO_EFFECT);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDMOWrapperFilter::Init(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    wrapper_filter_p->Release ();
    wrapper_filter_p = NULL;

    goto error;
  } // end IF
  // set effect options
  if (effect_in == GUID_DSCFX_CLASS_AEC)
  {

  } // end IF
  //////////////////////////////////////
  else if (effect_in == GUID_DSFX_STANDARD_CHORUS)
  {
    IDirectSoundFXChorus* chorus_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXChorus,
                                               (void**)&chorus_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXChorus): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      wrapper_filter_p->Release ();
      wrapper_filter_p = NULL;

      goto error;
    } // end IF
    result = chorus_p->SetAllParameters (&effectOptions_in.chorusOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXChorus::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      chorus_p->Release ();
      wrapper_filter_p->Release ();
      wrapper_filter_p = NULL;

      goto error;
    } // end IF
    chorus_p->Release ();
  } // end ELSE IF
  else if (effect_in == GUID_DSFX_STANDARD_COMPRESSOR)
  {

  } // end ELSE IF
  else if (effect_in == GUID_DSFX_STANDARD_DISTORTION)
  {

  } // end ELSE IF
  else if (effect_in == GUID_DSFX_STANDARD_ECHO)
  {
    IDirectSoundFXEcho* echo_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXEcho,
                                               (void**)&echo_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXEcho): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      wrapper_filter_p->Release ();
      wrapper_filter_p = NULL;

      goto error;
    } // end IF
    result = echo_p->SetAllParameters (&effectOptions_in.echoOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXEcho::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      echo_p->Release ();
      wrapper_filter_p->Release ();
      wrapper_filter_p = NULL;

      goto error;
    } // end IF
    echo_p->Release ();
  } // end ELSE IF
  else if (effect_in == GUID_DSFX_STANDARD_PARAMEQ)
  {

  } // end ELSE IF
  else if (effect_in == GUID_DSFX_STANDARD_FLANGER)
  {

  } // end ELSE IF
  else if (effect_in == GUID_DSFX_STANDARD_GARGLE)
  {

  } // end ELSE IF
  else if (effect_in == GUID_DSFX_STANDARD_I3DL2REVERB)
  {

  } // end ELSE IF
  else if (effect_in == GUID_DSFX_WAVES_REVERB)
  {

  } // end ELSE IF
  //////////////////////////////////////
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown effect (was: \"%s\"), continuing\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (effect_in).c_str ())));
  wrapper_filter_p->Release ();
  wrapper_filter_p = NULL;
  result = IGraphBuilder_in->AddFilter (filter_3,
                                        MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO)));

continue_:
  // send to an output (waveOut) ?
  if (audioOutput_in > 0)
  {
    GUID_s = CLSID_AudioRender;
    graph_entry.filterName = MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO;
  } // end IF
  else
  {
    GUID_s = CLSID_NullRenderer;
    graph_entry.filterName = MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL;
  } // end ELSE
  result = CoCreateInstance (GUID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_4));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (GUID_s).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_4);
  result =
    IGraphBuilder_in->AddFilter (filter_4,
                                 graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

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

  graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB;
  //pipeline_out.push_back (converter_name);
  graph_out.push_back (graph_entry);
  if (effect_in != GUID_NULL)
  {
    graph_entry.filterName = MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO;
    graph_out.push_back (graph_entry);
  } // end IF
  graph_entry.filterName =
    ((audioOutput_in > 0) ? MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                          : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL);
  graph_out.push_back (graph_entry);

  // clean up
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (filter_4)
    filter_4->Release ();

  return true;

error:
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (filter_4)
    filter_4->Release ();

  return false;
}
bool
Stream_Module_Device_DirectShow_Tools::loadVideoRendererGraph (REFGUID deviceCategory_in,
                                                               const struct _AMMediaType& mediaType_in,
                                                               const HWND windowHandle_in,
                                                               IGraphBuilder* IGraphBuilder_in,
                                                               Stream_Module_Device_DirectShow_Graph_t& graph_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::loadVideoRendererGraph"));

  HRESULT result = E_FAIL;
  struct Stream_Module_Device_DirectShow_GraphEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  IPin* pin_p = NULL;
  bool result_2 = false;
  bool skip_decode = false;
  bool skip_grab = false;
  bool is_partially_connected = false;
  struct _GUID CLSID_s;
  struct _GUID preferred_subtype = GUID_NULL;
  FOURCCMap fourcc_map;
  struct _AMMediaType* media_type_p = NULL;

  // initialize return value(s)
  for (Stream_Module_Device_DirectShow_GraphIterator_t iterator = graph_out.begin ();
       iterator != graph_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_Module_Device_DirectShow_Tools::deleteMediaType ((*iterator).mediaType);
  graph_out.clear ();

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  if (deviceCategory_in == CLSID_AudioInputDeviceCategory)
    graph_entry.filterName =
      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (deviceCategory_in == CLSID_VideoInputDeviceCategory)
    graph_entry.filterName =
      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (deviceCategory_in).c_str ())));
    goto error;
  } // end ELSE

  if (!Stream_Module_Device_DirectShow_Tools::resetGraph (IGraphBuilder_in,
                                                          deviceCategory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::resetGraph(), aborting\n")));
    goto error;
  } // end IF
  if (Stream_Module_Device_DirectShow_Tools::has (IGraphBuilder_in,
                                                  graph_entry.filterName))
  {
    if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in,
                                                               graph_entry.mediaType))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n")));
      goto error;
    } // end IF
    graph_out.push_back (graph_entry);
    graph_entry.mediaType = NULL;
  } // end IF

  // step1: decompress ?
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in,
                                                             graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (graph_entry.mediaType);
  fourcc_map.SetFOURCC (&graph_entry.mediaType->subtype);

  if (!Stream_Module_Device_Tools::isCompressedVideo (graph_entry.mediaType->subtype,
                                                      false))                         // media foundation application ?
    goto decode;

decompress:
  switch (fourcc_map.GetFOURCC ())
  {
    // *** compressed types ***
    case FCC ('H264'):
    {
      CLSID_s = CLSID_CMPEG2VidDecoderDS;
      graph_entry.filterName =
        MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264;
      preferred_subtype = MEDIASUBTYPE_NV12;
      // *NOTE*: the EVR video renderer (!) can handle the nv12 chroma type
      //         --> do not decode
      skip_decode = true;
      // *TODO*: for some reason the decoder fails to connect to the sample
      //          grabber
      //         --> do not grab
      skip_grab = true;
      break;
    }
    case FCC ('MJPG'):
    {
      CLSID_s = CLSID_MjpegDec;
      graph_entry.filterName =
        MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype).c_str ())));
      goto error;
    }
  } // end SWITCH

  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

  pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                      PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                graph_entry.filterName.c_str ()));
    goto error;
  } // end IF
#if defined (_DEBUG)
  Stream_Module_Device_DirectShow_Tools::countFormats (pin_p,
                                                       graph_entry.mediaType->formattype);
#endif
  filter_p->Release ();
  filter_p = NULL;

  if (preferred_subtype != GUID_NULL)
  {
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (graph_entry.mediaType);
    graph_entry.mediaType = NULL;

    if (!Stream_Module_Device_DirectShow_Tools::getFirstFormat (pin_p,
                                                                preferred_subtype,
                                                                graph_entry.mediaType))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to Stream_Module_Device_DirectShow_Tools::getFirstFormat(\"%s\"), aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                  ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (preferred_subtype).c_str ())));
      goto error;
    } // end IF
  } // end IF
  graph_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  // need another decompressor ?
  // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
  //         (TM) AVI/MJPG decoders) when the filter is not connected
  //         --> (partially) connect the graph (and retry)
  if (!Stream_Module_Device_DirectShow_Tools::countFormats (pin_p,
                                                            graph_out.back ().mediaType->formattype))
  {
    if (!is_partially_connected)
    {
      is_partially_connected = true;

      if (!Stream_Module_Device_DirectShow_Tools::connect (IGraphBuilder_in,
                                                           graph_out))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::connect(), aborting\n")));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("partially connected the graph...\n")));
    } // end IF
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::hasUncompressedFormat (CLSID_VideoInputDeviceCategory,
                                                                     pin_p,
                                                                     graph_entry.mediaType))
  { ACE_ASSERT (!graph_entry.mediaType);
    if (!Stream_Module_Device_DirectShow_Tools::getFirstFormat (pin_p,
                                                                GUID_NULL,
                                                                graph_entry.mediaType))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to Stream_Module_Device_DirectShow_Tools::getFirstFormat(), aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));
      goto error;
    } // end IF
    pin_p->Release ();
    pin_p = NULL;
    ACE_ASSERT (graph_entry.mediaType);

    goto decompress;
  } // end IF
  if (is_partially_connected)
  {
    is_partially_connected = false;

    if (!Stream_Module_Device_DirectShow_Tools::disconnect (IGraphBuilder_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("disconnected partially connected graph...\n")));
  } // end IF
  ACE_ASSERT (graph_entry.mediaType);
  pin_p->Release ();
  pin_p = NULL;

decode:
  if (skip_decode) goto grab;

  preferred_subtype = MEDIASUBTYPE_RGB24;
  if (Stream_Module_Decoder_Tools::isRGB (graph_entry.mediaType->subtype,
                                          false))                         // media foundation application ?
  {
    CLSID_s = CLSID_Colour;
    graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  } // end IF
  else if (Stream_Module_Decoder_Tools::isChromaLuminance (graph_entry.mediaType->subtype,
                                                           false))                         // media foundation application ?
  {
    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
    CLSID_s = CLSID_AVIDec;
    graph_entry.filterName =
      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
  } // end ELSE IF
  else
  //FOURCCMap fourcc_map_2 (&media_type_p->subtype);
  //switch (fourcc_map_2.GetFOURCC ())
  //{
  //  // *** uncompressed types ***
  //  // RGB types
  //  case FCC ('RGBA'):
  //  {
  //    CLSID_s = CLSID_Colour;
  //    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  //    break;
  //  }
  //  // chroma-luminance types
  //  case FCC ('YUY2'):
  //  {
  //    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
  //    CLSID_s = CLSID_AVIDec;
  //    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
  //    break;
  //  }
  //  default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype).c_str ())));
      goto error;
    }
  //} // end SWITCH

  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

  if (preferred_subtype != GUID_NULL)
  {
    pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                        PINDIR_OUTPUT);
    if (!pin_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s has no output pin, aborting\n"),
                  graph_entry.filterName.c_str ()));
      goto error;
    } // end IF
    // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
    //         (TM) AVI/MJPG decoders) when the filter is not connected
    //         --> (partially) connect the graph (and retry)
    if (!Stream_Module_Device_DirectShow_Tools::countFormats (pin_p,
                                                              graph_entry.mediaType->formattype))
    {
      if (!is_partially_connected)
      {
        is_partially_connected = true;

        // *IMPORTANT NOTE*: revert this afterwards (see below)
        Stream_Module_Device_DirectShow_Tools::deleteMediaType (graph_entry.mediaType);
        graph_out.push_back (graph_entry);
        if (!Stream_Module_Device_DirectShow_Tools::connect (IGraphBuilder_in,
                                                             graph_out))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::connect(), aborting\n")));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("partially connected the graph...\n")));
      } // end IF
    } // end IF
    if (!Stream_Module_Device_DirectShow_Tools::getFirstFormat (pin_p,
                                                                preferred_subtype,
                                                                graph_entry.mediaType))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to Stream_Module_Device_DirectShow_Tools::getFirstFormat(\"%s\"), aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                  ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (preferred_subtype).c_str ())));
      goto error;
    } // end IF
    pin_p->Release ();
    pin_p = NULL;

    if (is_partially_connected)
    {
      is_partially_connected = false;

      if (!Stream_Module_Device_DirectShow_Tools::disconnect (IGraphBuilder_in))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(), aborting\n")));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("disconnected partially connected graph...\n")));
      graph_out.pop_back ();
    } // end IF
  } // end IF
  //ACE_ASSERT (graph_entry.mediaType);
  filter_p->Release ();
  filter_p = NULL;
  graph_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

grab:
  if (skip_grab) goto render;

  graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB;
  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_SampleGrabber).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  filter_p->Release ();
  filter_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));

  graph_out.push_back (graph_entry);

render:
  if (windowHandle_in)
  {
    //CLSID_s = CLSID_VideoRenderer;
    CLSID_s = CLSID_EnhancedVideoRenderer;
    graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO;
  } // end IF
  else
  {
    CLSID_s = CLSID_NullRenderer;
    graph_entry.filterName = MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL;
  } // end ELSE
  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  filter_p->Release ();
  filter_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

  graph_out.push_back (graph_entry);

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

  return true;

error:
  for (Stream_Module_Device_DirectShow_GraphIterator_t iterator = graph_out.begin ();
       iterator != graph_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_Module_Device_DirectShow_Tools::deleteMediaType ((*iterator).mediaType);
  graph_out.clear ();
  if (pin_p)
    pin_p->Release ();
  if (filter_p)
    filter_p->Release ();
  if (graph_entry.mediaType)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (graph_entry.mediaType);

  return false;
}

bool
Stream_Module_Device_DirectShow_Tools::loadTargetRendererGraph (IBaseFilter* sourceFilter_in,
                                                                const std::wstring& sourceFilterName_in,
                                                                const struct _AMMediaType& mediaType_in,
                                                                const HWND windowHandle_in,
                                                                IGraphBuilder*& IGraphBuilder_out,
                                                                IAMBufferNegotiation*& IAMBufferNegotiation_out,
                                                                Stream_Module_Device_DirectShow_Graph_t& graph_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::loadTargetRendererGraph"));

  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;
  IBaseFilter* filter_2 = NULL;
  IBaseFilter* filter_3 = NULL;
  IPin* pin_p = NULL;
  struct Stream_Module_Device_DirectShow_GraphEntry graph_entry;
  FOURCCMap fourcc_map;
  bool skip_decode = false;
  bool skip_resize = false;
  bool is_partially_connected = false;
  struct _GUID CLSID_s, CLSID_2;
  struct _GUID preferred_subtype = GUID_NULL;
  bool filter_is_dmo_wrapper = false;
  struct _AMMediaType* media_type_p = NULL;
  IDMOWrapperFilter* i_dmo_wrapper_filter_p = NULL;
  struct _DMOMediaType* dmo_media_type_p = NULL;
  IMediaObject* i_media_object_p = NULL;
  IWMResizerProps* i_wmresizer_props_p = NULL;
  DWORD dwFlags = 0;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;

  // initialize return value(s)
  for (Stream_Module_Device_DirectShow_GraphIterator_t iterator = graph_out.begin ();
       iterator != graph_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_Module_Device_DirectShow_Tools::deleteMediaType ((*iterator).mediaType);
  graph_out.clear ();

  if (!IGraphBuilder_out)
  {
    result =
      CoCreateInstance (CLSID_FilterGraph, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&IGraphBuilder_out));
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
    if (!Stream_Module_Device_DirectShow_Tools::clear (IGraphBuilder_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (!IAMBufferNegotiation_out);

  // instantiate source filter ?
  if (!sourceFilter_in)
  {
    // sanity check(s)
    if (!Stream_Module_Device_DirectShow_Tools::has (IGraphBuilder_out,
                                                     sourceFilterName_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: not part of the filter graph, aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ())));
      goto error;
    } // end IF

    if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in,
                                                               graph_entry.mediaType))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (graph_entry.mediaType);

    goto continue_;
  } // end IF

  pin_p = Stream_Module_Device_DirectShow_Tools::pin (sourceFilter_in,
                                                      PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (sourceFilter_in).c_str ())));
    goto error;
  } // end IF
  if (!Stream_Module_Device_DirectShow_Tools::getFirstFormat (pin_p,
                                                              GUID_NULL,
                                                              graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_Module_Device_DirectShow_Tools::getFirstFormat(), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (sourceFilter_in).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));
    goto error;
  } // end IF
  pin_p->Release ();
  pin_p = NULL;

  result = IGraphBuilder_out->AddFilter (sourceFilter_in,
                                         sourceFilterName_in.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  graph_entry.filterName = sourceFilterName_in;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ())));

continue_:
  graph_out.push_back (graph_entry);

  unsigned int source_width, width, source_height, height;
  if (graph_entry.mediaType->formattype == FORMAT_VideoInfo)
  { ACE_ASSERT (mediaType_in.formattype == FORMAT_VideoInfo);
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (graph_entry.mediaType->pbFormat);
    struct tagVIDEOINFOHEADER* video_info_header_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
    source_height = abs (video_info_header_p->bmiHeader.biHeight);
    source_width = video_info_header_p->bmiHeader.biWidth;
    height = abs (video_info_header_2->bmiHeader.biHeight);
    width = video_info_header_2->bmiHeader.biWidth;
  } // end IF
  else if (graph_entry.mediaType->formattype == FORMAT_VideoInfo2)
  { ACE_ASSERT (mediaType_in.formattype == FORMAT_VideoInfo2);
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (graph_entry.mediaType->pbFormat);
    struct tagVIDEOINFOHEADER2* video_info_header2_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_in.pbFormat);
    source_height = abs (video_info_header2_p->bmiHeader.biHeight);
    source_width = video_info_header2_p->bmiHeader.biWidth;
    height = abs (video_info_header2_2->bmiHeader.biHeight);
    width = video_info_header2_2->bmiHeader.biWidth;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown format type (was: %s), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (graph_entry.mediaType->formattype).c_str ())));
    goto error;
  } // end ELSE
  skip_resize = (source_height == height) && (source_width == width);

  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*graph_entry.mediaType,
                                                             media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  graph_entry.mediaType = media_type_p;
  media_type_p = NULL;

  if (!Stream_Module_Device_Tools::isCompressedVideo (graph_entry.mediaType->subtype,
                                                      false))                         // media foundation application ?
    goto decode;

decompress:
  fourcc_map.SetFOURCC (&graph_entry.mediaType->subtype);
  switch (fourcc_map.GetFOURCC ())
  {
    // *** compressed types ***
    case FCC ('H264'):
    {
      CLSID_s = CLSID_CMPEG2VidDecoderDS;
      graph_entry.filterName =
        MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264;
      // *NOTE*: the EVR video renderer (!) can handle the nv12 chroma type
      //         --> do not decode
      preferred_subtype = MEDIASUBTYPE_NV12;
      skip_decode = true;
      break;
    }
    case FCC ('MJPG'):
    {
      CLSID_s = CLSID_MjpegDec;
      graph_entry.filterName =
        MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype).c_str ())));
      goto error;
    }
  } // end SWITCH

  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_out->AddFilter (filter_p,
                                         graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

  pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                      PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                graph_entry.filterName.c_str ()));
    goto error;
  } // end IF
  filter_p->Release ();
  filter_p = NULL;
  graph_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  // need another decompressor ?
  // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
  //         (TM) AVI/MJPG decoders) when the filter is not connected
  //         --> (partially) connect the graph (and retry)
  if (!Stream_Module_Device_DirectShow_Tools::countFormats (pin_p,
                                                            graph_out.back ().mediaType->formattype))
  {
    if (!is_partially_connected)
    {
      is_partially_connected = true;

      if (!Stream_Module_Device_DirectShow_Tools::connect (IGraphBuilder_out,
                                                           graph_out))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::connect(), aborting\n")));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("partially connected the graph...\n")));
    } // end IF
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::getFirstFormat (pin_p,
                                                              preferred_subtype,
                                                              graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_Module_Device_DirectShow_Tools::getFirstFormat(\"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (preferred_subtype).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graph_entry.mediaType);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: selected output format: %s...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*graph_entry.mediaType, true).c_str ())));

  if (Stream_Module_Device_Tools::isCompressedVideo (graph_entry.mediaType->subtype,
                                                     false))
    goto decompress;

  if (is_partially_connected)
  {
    is_partially_connected = false;

    if (!Stream_Module_Device_DirectShow_Tools::disconnect (IGraphBuilder_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("disconnected partially connected graph...\n")));
  } // end IF
  pin_p->Release ();
  pin_p = NULL;

decode:
  if (skip_decode)
    goto grab;

  preferred_subtype = MEDIASUBTYPE_RGB24;
  if (Stream_Module_Decoder_Tools::isRGB (graph_entry.mediaType->subtype,
                                          false))                         // media foundation application ?
  {
    CLSID_s = CLSID_Colour;
    graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  } // end IF
  else if (Stream_Module_Decoder_Tools::isChromaLuminance (graph_entry.mediaType->subtype,
                                                           false))                         // media foundation application ?
  {
    //// *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
    //CLSID_s = CLSID_AVIDec;
    CLSID_s = CLSID_DMOWrapperFilter;
    graph_entry.filterName =
      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_YUV;
    filter_is_dmo_wrapper = true;
    CLSID_2 = CLSID_CColorConvertDMO;
  } // end ELSE IF
  else
  //FOURCCMap fourcc_map_2 (&media_type_p->subtype);
  //switch (fourcc_map_2.GetFOURCC ())
  //{
  //  // *** uncompressed types ***
  //  // RGB types
  //  case FCC ('RGBA'):
  //  {
  //    CLSID_s = CLSID_Colour;
  //    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  //    break;
  //  }
  //  // chroma-luminance types
  //  case FCC ('YUY2'):
  //  {
  //    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
  //    CLSID_s = CLSID_AVIDec;
  //    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
  //    break;
  //  }
  //  default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype).c_str ())));
      goto error;
    }
  //} // end SWITCH

  // *TODO*: support receiving other formats
  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_2));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                graph_entry.filterName.c_str (),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_2);
  if (filter_is_dmo_wrapper)
  {
    result = filter_2->QueryInterface (IID_PPV_ARGS (&i_dmo_wrapper_filter_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IDMOWrapperFilter): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (i_dmo_wrapper_filter_p);
    result = i_dmo_wrapper_filter_p->Init (CLSID_2,
                                           DMOCATEGORY_VIDEO_DECODER);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::Init(DMOCATEGORY_VIDEO_DECODER): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      i_dmo_wrapper_filter_p->Release ();

      goto error;
    } // end IF

    // set input type manually
    // *NOTE*: DMO_MEDIA_TYPE is actually a typedef for AM_MEDIA_TYPE, so this
    //         creates a copy
    if (!Stream_Module_Device_DirectShow_Tools::AMMediaTypeToDMOMediaType (*graph_entry.mediaType,
                                                                           dmo_media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_Common_DirectShow_Tools::AMMediaTypeToDMOMediaType(), aborting\n")));

      // clean up
      i_dmo_wrapper_filter_p->Release ();

      goto error;
    } // end IF
    ACE_ASSERT (dmo_media_type_p);
    result = filter_2->QueryInterface (IID_PPV_ARGS (&i_media_object_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IMediaObject): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      i_dmo_wrapper_filter_p->Release ();
      DeleteMediaType ((struct _AMMediaType*)dmo_media_type_p);

      goto error;
    } // end IF
    ACE_ASSERT (i_dmo_wrapper_filter_p);
    result = i_media_object_p->SetInputType (0,
                                             dmo_media_type_p,
                                             dwFlags);
    if (FAILED (result)) // E_INVALIDARG: 0x80070057
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMediaObject::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      DeleteMediaType ((struct _AMMediaType*)dmo_media_type_p);
      i_media_object_p->Release ();
      i_dmo_wrapper_filter_p->Release ();

      goto error;
    } // end IF
    DeleteMediaType ((struct _AMMediaType*)dmo_media_type_p);
    i_media_object_p->Release ();
    i_dmo_wrapper_filter_p->Release ();
  } // end IF
  result =
    IGraphBuilder_out->AddFilter (filter_2,
                                  graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
  ACE_ASSERT (filter_2);

  pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_2,
                                                      PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                graph_entry.filterName.c_str ()));
    goto error;
  } // end IF
  filter_2->Release ();
  filter_2 = NULL;
  graph_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
  //         (TM) AVI/MJPG decoders) when the filter is not connected
  //         --> (partially) connect the graph (and retry)
  if (!Stream_Module_Device_DirectShow_Tools::countFormats (pin_p,
                                                            graph_out.back ().mediaType->formattype))
  {
    if (!is_partially_connected)
    {
      is_partially_connected = true;

      if (!Stream_Module_Device_DirectShow_Tools::connect (IGraphBuilder_out,
                                                           graph_out))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::connect(), aborting\n")));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("partially connected the graph...\n")));
    } // end IF
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::getFirstFormat (pin_p,
                                                              preferred_subtype,
                                                              graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_Module_Device_DirectShow_Tools::getFirstFormat(\"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (preferred_subtype).c_str ())));
#if defined (_DEBUG)
    Stream_Module_Device_DirectShow_Tools::countFormats (pin_p,
                                                         GUID_NULL);
#endif

    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: selected output format: %s...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*graph_entry.mediaType, true).c_str ())));
  pin_p->Release ();
  pin_p = NULL;
  ACE_ASSERT (graph_entry.mediaType);

  if (is_partially_connected)
  {
    is_partially_connected = false;

    if (!Stream_Module_Device_DirectShow_Tools::disconnect (IGraphBuilder_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("disconnected partially connected graph...\n")));
  } // end IF

  if (!Stream_Module_Decoder_Tools::isRGB (graph_entry.mediaType->subtype,
                                           false))
    goto decode;

resize:
  if (skip_resize)
    goto grab;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("scaling video %ux%u --> %ux%u...\n"),
              source_width, source_height, width, height));

  CLSID_s = CLSID_DMOWrapperFilter;
  CLSID_2 = CLSID_CResizerDMO;
  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_2));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                graph_entry.filterName.c_str (),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_2);
  result = filter_2->QueryInterface (IID_PPV_ARGS (&i_dmo_wrapper_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IDMOWrapperFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (i_dmo_wrapper_filter_p);
  result = i_dmo_wrapper_filter_p->Init (CLSID_2, DMOCATEGORY_VIDEO_EFFECT);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDMOWrapperFilter::Init(DMOCATEGORY_VIDEO_EFFECT): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    i_dmo_wrapper_filter_p->Release ();

    goto error;
  } // end IF
  i_dmo_wrapper_filter_p->Release ();
  i_dmo_wrapper_filter_p = NULL;

  result = filter_2->QueryInterface (IID_PPV_ARGS (&i_wmresizer_props_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IWMResizerProps): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (i_wmresizer_props_p);
  result =
    i_wmresizer_props_p->SetFullCropRegion (0, 0, source_width, source_height,
                                            0, 0, width, height);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IWMResizerProps::SetFullCropRegion(0,0,%u,%u,0,0,%u,%u): \"%s\", aborting\n"),
                source_width, source_height,
                width, height,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    i_wmresizer_props_p->Release ();

    goto error;
  } // end IF
  result = i_wmresizer_props_p->SetInterlaceMode (FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = i_wmresizer_props_p->SetResizerQuality (FALSE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IWMResizerProps::SetResizerQuality(false): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    i_wmresizer_props_p->Release ();

    goto error;
  } // end IF
  i_wmresizer_props_p->Release ();
  i_wmresizer_props_p = NULL;

  // set input type manually
  // *NOTE*: DMO_MEDIA_TYPE is actually a typedef for AM_MEDIA_TYPE, so this
  //         creates a copy
  dmo_media_type_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::AMMediaTypeToDMOMediaType (*graph_entry.mediaType,
                                                                         dmo_media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Common_DirectShow_Tools::AMMediaTypeToDMOMediaType(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (dmo_media_type_p);
  result = filter_2->QueryInterface (IID_PPV_ARGS (&i_media_object_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IMediaObject): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    DeleteMediaType ((struct _AMMediaType*)dmo_media_type_p);

    goto error;
  } // end IF
  ACE_ASSERT (i_media_object_p);
  result = i_media_object_p->SetInputType (0,
                                           dmo_media_type_p,
                                           dwFlags);
  if (FAILED (result)) // E_INVALIDARG: 0x80070057
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaObject::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    DeleteMediaType ((struct _AMMediaType*)dmo_media_type_p);
    i_media_object_p->Release ();
    i_media_object_p = NULL;

    goto error;
  } // end IF
  DeleteMediaType ((struct _AMMediaType*)dmo_media_type_p);
  dmo_media_type_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::AMMediaTypeToDMOMediaType (mediaType_in,
                                                                         dmo_media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Common_DirectShow_Tools::AMMediaTypeToDMOMediaType(), aborting\n")));

    // clean up
    i_media_object_p->Release ();
    i_media_object_p = NULL;

    goto error;
  } // end IF
  ACE_ASSERT (dmo_media_type_p);
  ACE_ASSERT (dmo_media_type_p->pbFormat);
  if (dmo_media_type_p->formattype == FORMAT_VideoInfo)
  {
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (dmo_media_type_p->pbFormat);
    SetRect (&video_info_header_p->rcSource,
             0, 0, source_width, source_height);
    SetRect (&video_info_header_p->rcTarget,
             0, 0, width, height);
  } // end IF
  else if (dmo_media_type_p->formattype == FORMAT_VideoInfo2)
  {
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (dmo_media_type_p->pbFormat);
    SetRect (&video_info_header2_p->rcSource,
             0, 0, source_width, source_height);
    SetRect (&video_info_header2_p->rcTarget,
             0, 0, width, height);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown format type (was: %s), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (dmo_media_type_p->formattype).c_str ())));
    goto error;
  } // end ELSE
  result = i_media_object_p->SetOutputType (0,
                                            dmo_media_type_p,
                                            dwFlags);
  if (FAILED (result)) // E_INVALIDARG: 0x80070057
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaObject::SetOutputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    DeleteMediaType ((struct _AMMediaType*)dmo_media_type_p);
    i_media_object_p->Release ();
    i_media_object_p = NULL;

    goto error;
  } // end IF
  i_media_object_p->Release ();
  i_media_object_p = NULL;

  graph_entry.filterName =
    MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RESIZER_VIDEO;
  result =
    IGraphBuilder_out->AddFilter (filter_2,
                                  graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
  ACE_ASSERT (filter_2);

  graph_out.push_back (graph_entry);
  graph_entry.connectDirect = true;
  graph_entry.mediaType =
    reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: set output format: %s...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*graph_entry.mediaType, true).c_str ())));

  // *TODO*: implement frame grabber functionality
grab:

render:
  // render to a window (e.g. GtkDrawingArea) ?
  graph_entry.filterName =
    (windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
                     : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL);

  result =
    IGraphBuilder_out->FindFilterByName (graph_entry.filterName.c_str (),
                                         &filter_3);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    CLSID_s =
      (windowHandle_in ? MODULE_DEV_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER
                       : CLSID_NullRenderer);
    result = CoCreateInstance (CLSID_s, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_3));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (CLSID_s).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_3);
    result =
      IGraphBuilder_out->AddFilter (filter_3,
                                    graph_entry.filterName.c_str ());
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
  } // end IF
  ACE_ASSERT (filter_3);
  filter_3->Release ();
  filter_3 = NULL;
  graph_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

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

  // clean up
  //if (filter_p)
  //  filter_p->Release ();

  if (!Stream_Module_Device_DirectShow_Tools::getBufferNegotiation (IGraphBuilder_out,
                                                                    sourceFilterName_in,
                                                                    IAMBufferNegotiation_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::getBufferNegotiation(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);

#if defined (_DEBUG)
  Stream_Module_Device_DirectShow_Tools::dump (graph_out);
#endif

  return true;

error:
  //if (filter_p)
  //  filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (pin_p)
    pin_p->Release ();

  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release ();
    IGraphBuilder_out = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Device_DirectShow_Tools::connect (IGraphBuilder* builder_in,
                                               const Stream_Module_Device_DirectShow_Graph_t& graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::connect"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!graph_in.empty ());
  //if (!Stream_Module_Device_DirectShow_Tools::disconnect (builder_in))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(), aborting\n")));
  //  return false;
  //} // end IF

  IBaseFilter* filter_p = NULL;
  Stream_Module_Device_DirectShow_GraphConstIterator_t iterator =
    graph_in.begin ();
  HRESULT result =
    builder_in->FindFilterByName ((*iterator).filterName.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  //IAMStreamConfig* stream_config_p = NULL;
  IPin* pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                            PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  filter_p->Release ();

  IAMStreamConfig* stream_config_p = NULL;
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IPin::QueryInterface(IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (stream_config_p);
  result =
    stream_config_p->SetFormat ((*iterator).mediaType); // 'NULL' should reset the pin
  if (FAILED (result))
  {
    if ((*iterator).mediaType)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IAMStreamConfig::SetFormat(): \"%s\" (media type was: %s), continuing\n"),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ()),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*(*iterator).mediaType).c_str ())));
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IAMStreamConfig::SetFormat(NULL): \"%s\", continuing\n"),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF
  //else
  //{
  //  if ((*iterator).mediaType)
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("%s: set capture format: %s\n"),
  //                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
  //                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*(*iterator).mediaType, true).c_str ())));
  //  else
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("%s: reset capture format...\n"),
  //                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ())));
  //} // end ELSE
  stream_config_p->Release ();

  IPin* pin_2 = NULL;
  IPin* pin_3 = NULL;
  Stream_Module_Device_DirectShow_GraphConstIterator_t iterator_2 = iterator;
  for (++iterator;
       iterator != graph_in.end ();
       ++iterator_2)
  {
    filter_p = NULL;
    result = builder_in->FindFilterByName ((*iterator).filterName.c_str (),
                                           &filter_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (filter_p);

    pin_2 = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                        PINDIR_INPUT);
    if (!pin_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: has no input pin, aborting\n"),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ())));

      // clean up
      filter_p->Release ();

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

      //if (result == VFW_E_NO_ACCEPTABLE_TYPES)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IPin::Connect() \"%s\", aborting\n"),
      //              ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).c_str ())));

      //  // debug info
      //  Stream_Module_Device_DirectShow_Tools::dump (pin_p);
      //  Stream_Module_Device_DirectShow_Tools::dump (pin_2);
      //} // end IF
      //else
      //{
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("'direct' pin connection %s/%s <--> %s/%s failed (media type was: %s): \"%s\" (0x%x), retrying...\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                    ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                    ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                    ((*iterator).mediaType ? ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*(*iterator).mediaType,
                                                                                                                 true).c_str ())
                                           : ACE_TEXT ("NULL")),
                    ACE_TEXT (Common_Tools::error2String (result, true).c_str ()),
                    result));
      //} // end ELSE

      result = builder_in->Connect (pin_p, pin_2);
      if (FAILED (result)) // 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
                           // 0x80040217: VFW_E_CANNOT_CONNECT
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("'intelligent' pin connection %s/%s <--> %s/%s failed: \"%s\" (0x%x), aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                    ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                    ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                    ACE_TEXT (Common_Tools::error2String (result, true).c_str ()),
                    result));
#if defined (_DEBUG)
        Stream_Module_Device_DirectShow_Tools::countFormats (pin_2,
                                                             GUID_NULL);
#endif
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
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ())));
//continue_2:
    pin_2->Release ();
    pin_2 = NULL;
    pin_p->Release ();

    if (++iterator != graph_in.end ())
    {
      pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
      if (!pin_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: has no output pin, aborting\n"),
                    ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ())));

        // clean up
        filter_p->Release ();

        break;
      } // end IF
    } // end IF
  } // end FOR

  return true;
}
bool
Stream_Module_Device_DirectShow_Tools::connectFirst (IGraphBuilder* builder_in,
                                                     const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::connectFirst"));

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  IPin* pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::pin(PINDIR_OUTPUT), aborting\n")));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF

  IPin* pin_2 = NULL;
loop:
  result = pin_p->ConnectedTo (&pin_2);
  if (FAILED (result))
  {
    filter_p = Stream_Module_Device_DirectShow_Tools::pin2Filter (pin_p);
    ACE_ASSERT (filter_p);
    result = builder_in->Render (pin_p);
    if (FAILED (result))
    {

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::Render(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
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

  filter_p = Stream_Module_Device_DirectShow_Tools::pin2Filter (pin_2);
  if (!filter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::pin2Filter(0x%@), aborting\n"),
                pin_2));

    // clean up
    pin_2->Release ();

    return false;
  } // end IF
  pin_2->Release ();
  pin_2 = NULL;

  pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
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
Stream_Module_Device_DirectShow_Tools::connected (IGraphBuilder* builder_in,
                                                  const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::connected"));

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  IPin* pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::pin(PINDIR_OUTPUT), aborting\n")));

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

  filter_p = Stream_Module_Device_DirectShow_Tools::pin2Filter (pin_2);
  if (!filter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::pin2Filter(0x%@), aborting\n"),
                pin_2));

    // clean up
    pin_2->Release ();

    return false;
  } // end IF
  pin_2->Release ();
  pin_2 = NULL;

  pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p, PINDIR_OUTPUT);
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
Stream_Module_Device_DirectShow_Tools::graphBuilderConnect (IGraphBuilder* builder_in,
                                                            const std::list<std::wstring>& graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::graphBuilderConnect"));

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
        Stream_Module_Device_DirectShow_Tools::dump (pin_p);
        // *TODO*: evidently, some filters do not expose their preferred media
        //         types (e.g. AVI Splitter), so the straight-forward, 'direct'
        //         pin connection algorithm (as implemented here) will not
        //         always work. Note how (such as in this example), this
        //         actually makes some sense, as 'container'- or other 'meta-'
        //         filters sometimes actually do not know (or care) about what
        //         kind of data they contain
        Stream_Module_Device_DirectShow_Tools::dump (pin_2);
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
Stream_Module_Device_DirectShow_Tools::clear (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::clear"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IEnumFilters* enumerator_p = NULL;
  HRESULT result = builder_in->EnumFilters (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
Stream_Module_Device_DirectShow_Tools::disconnect (IBaseFilter* filter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IEnumPins* enumerator_p = NULL;
  IPin* pin_p = NULL, *pin_2 = NULL;
  struct _FilterInfo filter_info;
  ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));

  HRESULT result = filter_in->QueryFilterInfo (&filter_info);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    enumerator_p->Release ();

    return false;
  } // end IF

  // clean up
  if (filter_info.pGraph)
    filter_info.pGraph->Release ();

  result = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);

  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
  {
    ACE_ASSERT (pin_p);

    pin_2 = NULL;
    result = pin_p->ConnectedTo (&pin_2);
    if (FAILED (result))
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF
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
      enumerator_p->Release ();

      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("disconnected \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName)));

    pin_p->Release ();
    pin_p = NULL;
  } // end WHILE
  enumerator_p->Release ();

  return true;
}

bool
Stream_Module_Device_DirectShow_Tools::disconnect (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::disconnect"));

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

    if (!Stream_Module_Device_DirectShow_Tools::disconnect (filter_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(%s), aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName)));

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
Stream_Module_Device_DirectShow_Tools::has (IGraphBuilder* builder_in,
                                            const std::wstring& filterName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::has"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  IBaseFilter* filter_p = NULL;
  HRESULT result = E_FAIL;

  result =
    builder_in->FindFilterByName (filterName_in.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filterName_in.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  filter_p->Release ();

  return true;
}

bool
Stream_Module_Device_DirectShow_Tools::resetGraph (IGraphBuilder* builder_in,
                                                   const struct _GUID& deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::resetGraph"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  std::wstring filter_name;
  IBaseFilter* filter_p = NULL;
  HRESULT result = E_FAIL;

  if (deviceCategory_in == CLSID_AudioInputDeviceCategory)
    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (deviceCategory_in == CLSID_VideoInputDeviceCategory)
    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else if (deviceCategory_in == GUID_NULL)
  { // retrieve the first filter that has no input pin
    IEnumFilters* enumerator_p = NULL;
    result = builder_in->EnumFilters (&enumerator_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    IPin* pin_p = NULL;
    struct _FilterInfo filter_info;
    while (enumerator_p->Next (1, &filter_p, NULL) == S_OK)
    {
      ACE_ASSERT (filter_p);

      pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
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
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

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
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return false;
  } // end ELSE

  if (!Stream_Module_Device_DirectShow_Tools::disconnect (builder_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(), aborting\n")));
    return false;
  } // end IF

  if (filter_name.empty ()) goto continue_;

  result =
    builder_in->FindFilterByName (filter_name.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

continue_:
  if (!Stream_Module_Device_DirectShow_Tools::clear (builder_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::clear(), aborting\n")));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF

  if (filter_name.empty ()) goto continue_2;

  result = builder_in->AddFilter (filter_p,
                                  filter_name.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  filter_p->Release ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ())));

continue_2:
  return true;
}

bool
Stream_Module_Device_DirectShow_Tools::getBufferNegotiation (IGraphBuilder* builder_in,
                                                             const std::wstring& filterName_in,
                                                             IAMBufferNegotiation*& IAMBufferNegotiation_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::getBufferNegotiation"));

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IPin* pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                            PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::pin(\"%s\",PINDIR_OUTPUT), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ())));

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);
  pin_p->Release ();

  return true;
}

std::string
Stream_Module_Device_DirectShow_Tools::mediaTypeToString2 (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::mediaTypeToString2"));

  std::string result;

  GUID2STRING_MAP_ITERATOR_T iterator =
    Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.find (mediaType_in.majortype);
  result = ACE_TEXT_ALWAYS_CHAR ("(maj/sub/fmt): ");
  if (iterator == Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.end ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media majortype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.majortype).c_str ())));
    result += Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.majortype);
  } // end IF
  else
    result += (*iterator).second;
  result += ACE_TEXT_ALWAYS_CHAR (" / ");
  result +=
    Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaType_in.subtype);
  result += ACE_TEXT_ALWAYS_CHAR (" / ");
  iterator =
    Stream_Module_Device_Tools::Stream_FormatType2StringMap.find (mediaType_in.formattype);
  if (iterator == Stream_Module_Device_Tools::Stream_FormatType2StringMap.end ())
  {
    if (mediaType_in.formattype != GUID_NULL) // <-- 'don't care'
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.formattype).c_str ())));
    result += Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.formattype);
  } // end IF
  else
    result += (*iterator).second;

  result += ACE_TEXT_ALWAYS_CHAR (" || (fixed/comp/size): ");
  std::ostringstream converter;
  converter << mediaType_in.bFixedSizeSamples;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR (" / ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.bTemporalCompression;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR (" / ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << mediaType_in.lSampleSize;
  result += converter.str ();

  if (mediaType_in.formattype == FORMAT_VideoInfo)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;

    result += ACE_TEXT_ALWAYS_CHAR (" || rates (bit/error/frame): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->dwBitRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->dwBitErrorRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
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
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biHeight;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biPlanes;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biBitCount;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
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
        Stream_Module_Decoder_Tools::FOURCCToString (video_info_header_p->bmiHeader.biCompression);
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header_p->bmiHeader.biSizeImage;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end IF
  else if (mediaType_in.formattype == FORMAT_VideoInfo2)
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;

    result += ACE_TEXT_ALWAYS_CHAR (" || rates (bit/error/frame): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwBitRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwBitErrorRate;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->AvgTimePerFrame;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR (" || flags (interlace/copyprotection): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwInterlaceFlags;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwCopyProtectFlags;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR (" || aspect ratio (x/y): ");

    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->dwPictAspectRatioX;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
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
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biHeight;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biPlanes;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biBitCount;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
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
        Stream_Module_Decoder_Tools::FOURCCToString (video_info_header2_p->bmiHeader.biCompression);
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << video_info_header2_p->bmiHeader.biSizeImage;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end ELSE IF
  else if (mediaType_in.formattype == FORMAT_WaveFormatEx)
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;

    result += ACE_TEXT_ALWAYS_CHAR (" || format: ");

    WORD2STRING_MAP_ITERATOR_T iterator =
      Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.find (waveformatex_p->wFormatTag);
    if (iterator == Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.end ())
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
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nSamplesPerSec;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nAvgBytesPerSec;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->nBlockAlign;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->wBitsPerSample;
    result += converter.str ();
    result += ACE_TEXT_ALWAYS_CHAR (" / ");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatex_p->cbSize;
    result += converter.str ();

    if (waveformatex_p->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
      WAVEFORMATEXTENSIBLE* waveformatextensible_p =
        (WAVEFORMATEXTENSIBLE*)mediaType_in.pbFormat;

      result += ACE_TEXT_ALWAYS_CHAR (" || extensible (spb/mask/sub): ");

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      // *TODO*: the second argument may not be entirely accurate
      if (Stream_Module_Device_Tools::isCompressedAudio (mediaType_in.subtype,
                                                         false))
        converter << waveformatextensible_p->Samples.wSamplesPerBlock;
      else
        converter << waveformatextensible_p->Samples.wValidBitsPerSample;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR (" / ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter <<
        std::hex << waveformatextensible_p->dwChannelMask << std::dec;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR (" / ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      GUID2STRING_MAP_ITERATOR_T iterator =
        Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.find (waveformatextensible_p->SubFormat);
      if (iterator == Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.end ())
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("invalid/unknown wave subformat (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
        result += Stream_Module_Decoder_Tools::GUIDToString (waveformatextensible_p->SubFormat);
      } // end IF
      else
        result += (*iterator).second;
      result += ACE_TEXT_ALWAYS_CHAR ("\"\n");
    } // end IF
    else
      result += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end ELSE IF
  else if (mediaType_in.formattype != GUID_NULL) // <-- 'don't care'
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.formattype).c_str ())));

  return result;
}

bool
Stream_Module_Device_DirectShow_Tools::getCaptureFormat (IGraphBuilder* builder_in,
                                                         struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  if (mediaType_out)
  {
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_out);
    mediaType_out = NULL;
  } // end IF

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO),
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
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO)));
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
Stream_Module_Device_DirectShow_Tools::getFormat (IPin* pin_in,
                                                  struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::getFormat"));

  // sanity check(s)
  ACE_ASSERT (pin_in);
  if (mediaType_out)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_out);
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

  HRESULT result = pin_in->ConnectionMediaType (mediaType_out);
  if (FAILED (result))
  {
    IBaseFilter* filter_p =
      Stream_Module_Device_DirectShow_Tools::pin2Filter (pin_in);
    ACE_ASSERT (filter_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IPin::ConnectionMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_in).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_out);

    return false;
  } // end IF

  return true;
}
bool
Stream_Module_Device_DirectShow_Tools::getOutputFormat (IGraphBuilder* builder_in,
                                                        const std::wstring& filterName_in,
                                                        struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);
  ACE_ASSERT (!filterName_in.empty ());
  if (mediaType_out)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_out);
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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  if (filterName_in == MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)
  {
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

    // *NOTE*: (reasons yet unknown,) connect()ing the sample grabber to the
    //         null renderer 'breaks' the connection between the AVI
    //         decompressor and the sample grabber (go ahead, try it in with
    //         graphedit.exe)
    result = isample_grabber_p->GetConnectedMediaType (mediaType_out);
    if (FAILED (result)) // 0x80040209: VFW_E_NOT_CONNECTED
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ISampleGrabber::GetConnectedMediaType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result, true).c_str ())));
      goto error;
    } // end IF
    isample_grabber_p->Release ();
    isample_grabber_p = NULL;

    goto continue_;
  } // end IF

  pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                      PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no output pin, aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (pin_p);
  if (!Stream_Module_Device_DirectShow_Tools::getFormat (pin_p,
                                                         mediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_Module_Device_DirectShow_Tools::getFormat(), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));
    goto error;
  } // end IF
  pin_p->Release ();
  pin_p = NULL;

continue_:
  filter_p->Release ();
  filter_p = NULL;

  return true;

error:
  if (mediaType_out)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_out);
  if (filter_p) filter_p->Release ();
  if (pin_p) pin_p->Release ();

  return false;
}

bool
Stream_Module_Device_DirectShow_Tools::getFirstFormat (IPin* pin_in,
                                                       REFGUID mediaSubType_in,
                                                       struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::getFirstFormat"));

  // sanity check(s)
  ACE_ASSERT (pin_in);
  ACE_ASSERT (!mediaType_inout);

  IEnumMediaTypes* enumerator_p = NULL;
  HRESULT result = pin_in->EnumMediaTypes (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
    if (!SUCCEEDED (result) ||
        (result == S_FALSE)) break;

    // sanity check(s)
    ACE_ASSERT (media_types_a[0]);

    if ((mediaSubType_in == GUID_NULL) ||
        (mediaSubType_in == media_types_a[0]->subtype)) break;

    Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_types_a[0]);
  } while (true);
  enumerator_p->Release ();

  if (media_types_a[0])
    mediaType_inout = media_types_a[0];

  return (mediaType_inout != NULL);
}
bool
Stream_Module_Device_DirectShow_Tools::hasUncompressedFormat (REFGUID deviceCategory_in,
                                                              IPin* pin_in,
                                                              struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::hasUncompressedFormat"));

  // sanity check(s)
  ACE_ASSERT (pin_in);
  ACE_ASSERT (!mediaType_inout);

  IEnumMediaTypes* enumerator_p = NULL;
  HRESULT result = pin_in->EnumMediaTypes (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
    if (!SUCCEEDED (result)) break;
  
    // sanity check(s)
    ACE_ASSERT (media_types_a[0]);

    if (!Stream_Module_Device_Tools::isCompressed (media_types_a[0]->subtype,
                                                   deviceCategory_in,
                                                   false))
      break;

    Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_types_a[0]);
  } while (true);
  enumerator_p->Release ();

  mediaType_inout = media_types_a[0];

  return (mediaType_inout != NULL);
}

unsigned int
Stream_Module_Device_DirectShow_Tools::countFormats (IPin* pin_in,
                                                     REFGUID formatType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::countFormats"));

  unsigned int result = 0;

  // sanity check(s)
  ACE_ASSERT (pin_in);

  IEnumMediaTypes* enumerator_p = NULL;
  HRESULT result_2 = pin_in->EnumMediaTypes (&enumerator_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return result;
  } // end IF

  struct _AMMediaType* media_types_a[1];
  ACE_OS::memset (media_types_a, 0, sizeof (media_types_a));
  ULONG fetched = 0;
  while (S_OK == enumerator_p->Next (1,
                                     media_types_a,
                                     &fetched))
  {
    // sanity check(s)
    ACE_ASSERT (media_types_a[0]);

    if ((formatType_in != GUID_NULL) &&
        (formatType_in != media_types_a[0]->formattype)) goto continue_;

    ++result;
#if defined (_DEBUG)
    Stream_Module_Device_DirectShow_Tools::dump (*media_types_a[0]);
#endif

continue_:
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_types_a[0]);
  } // end WHILE
  enumerator_p->Release ();

  return result;
}
void
Stream_Module_Device_DirectShow_Tools::listCaptureFormats (IBaseFilter* filter_in,
                                                           REFGUID formatType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::listCaptureFormats"));

  HRESULT result = E_FAIL;
  IEnumPins* enumerator_p = NULL;
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
  DWORD returned_size = 0;

  result = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (enumerator_p);

  while (enumerator_p->Next (1, &pin_p, NULL) == S_OK)
  {
    ACE_ASSERT (pin_p);

    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();

      return;
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
                  ACE_TEXT ("failed to IPin::QueryInterface(IKsPropertySet): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();

      return;
    } // end IF
    ACE_ASSERT (property_set_p);
    result = property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                                  NULL, 0,
                                  &GUID_s, sizeof (struct _GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      property_set_p->Release ();
      pin_p->Release ();
      enumerator_p->Release ();

      return;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (struct _GUID));
    if (GUID_s == PIN_CATEGORY_CAPTURE)
    {
      property_set_p->Release ();
      break;
    } // end IF

    property_set_p->Release ();
    pin_p->Release ();
    pin_p = NULL;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no capture pin found, returning\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_in).c_str ())));
    return;
  } // end IF

  IAMStreamConfig* stream_config_p = NULL;
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  pin_p->Release ();
  ACE_ASSERT (stream_config_p);

  int count, size;
  result = stream_config_p->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    stream_config_p->Release ();

    return;
  } // end IF

  struct _AMMediaType* media_type_p = NULL;
  BYTE audio_SCC[sizeof (struct _AUDIO_STREAM_CONFIG_CAPS)];
  BYTE video_SCC[sizeof (struct _VIDEO_STREAM_CONFIG_CAPS)];
  BYTE* SCC_p = NULL;
  if (size == sizeof (struct _AUDIO_STREAM_CONFIG_CAPS))
    SCC_p = audio_SCC;
  else if (size == sizeof (struct _VIDEO_STREAM_CONFIG_CAPS))
    SCC_p = video_SCC;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (IAMStreamConfig::GetNumberOfCapabilities() returned size: %d), returning\n"),
                size));

    // clean up
    stream_config_p->Release ();

    return;
  } // end ELSE
  struct _AUDIO_STREAM_CONFIG_CAPS* audio_stream_config_caps_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS* video_stream_config_caps_p = NULL;
  for (int i = 0;
        i < count;
        ++i)
  {
    result = stream_config_p->GetStreamCaps (i,
                                             &media_type_p,
                                             SCC_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", returning\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      stream_config_p->Release ();

      return;
    } // end IF
    ACE_ASSERT (media_type_p);
    if ((formatType_in != GUID_NULL) &&
        (formatType_in != media_type_p->formattype))
    {
      Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    ACE_ASSERT (media_type_p->pbFormat);

    if (size == sizeof (struct _AUDIO_STREAM_CONFIG_CAPS))
    {
      audio_stream_config_caps_p =
        reinterpret_cast<struct _AUDIO_STREAM_CONFIG_CAPS*> (SCC_p);

      ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);

      Stream_Module_Device_DirectShow_Tools::dump (*media_type_p);
    } // end IF
    else if (size == sizeof (struct _VIDEO_STREAM_CONFIG_CAPS))
    {
      video_stream_config_caps_p =
        reinterpret_cast<struct _VIDEO_STREAM_CONFIG_CAPS*> (SCC_p);

      Stream_Module_Device_DirectShow_Tools::dump (*media_type_p);
    } // end ELSE
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown device category (IAMStreamConfig::GetNumberOfCapabilities() returned size: %d), returning\n"),
                  size));

      // clean up
      stream_config_p->Release ();
      Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);

      return;
    } // end ELSE

    Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
  } // end FOR
  stream_config_p->Release ();
}

bool
Stream_Module_Device_DirectShow_Tools::setCaptureFormat (IGraphBuilder* builder_in,
                                                         REFGUID deviceCategory_in,
                                                         const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::setCaptureFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (builder_in);

  std::wstring filter_name;
  if (deviceCategory_in == CLSID_AudioInputDeviceCategory)
    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (deviceCategory_in == CLSID_VideoInputDeviceCategory)
    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return false;
  } // end ELSE

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filter_name.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
#if defined (_DEBUG)
  Stream_Module_Device_DirectShow_Tools::listCaptureFormats (filter_p,
                                                             mediaType_in.formattype);
#endif

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
  enum _PinDirection pin_direction;
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
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
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
  if (FAILED (result)) // VFW_E_INVALIDMEDIATYPE: 0x80040200L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::SetFormat(): \"%s\" (0x%x) (media type was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ()), result,
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (mediaType_in).c_str ())));

    // clean up
    stream_config_p->Release ();

    return false;
  } // end IF
  stream_config_p->Release ();

  //struct _AMMediaType* media_type_p;
  //stream_config_p->GetFormat (&media_type_p);

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("set capture format: %s\n"),
  //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (mediaType_in, true).c_str ())));

  return true;
}

bool
Stream_Module_Device_DirectShow_Tools::copyMediaType (const struct _AMMediaType& mediaType_in,
                                                      struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::copyMediaType"));

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
    ACE_OS::memset (mediaType_out, 0, sizeof (struct _AMMediaType));
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
Stream_Module_Device_DirectShow_Tools::deleteMediaType (struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::deleteMediaType"));

  DeleteMediaType (mediaType_inout);
  mediaType_inout = NULL;
}

std::string
Stream_Module_Device_DirectShow_Tools::mediaTypeToString (const struct _AMMediaType& mediaType_in,
                                                          bool condensed_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_DirectShow_Tools::mediaTypeToString"));

  if (condensed_in)
    return Stream_Module_Device_DirectShow_Tools::mediaTypeToString2 (mediaType_in);

  std::string result;

  GUID2STRING_MAP_ITERATOR_T iterator =
    Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.find (mediaType_in.majortype);
  result = ACE_TEXT_ALWAYS_CHAR ("majortype: \"");
  if (iterator == Stream_Module_Device_DirectShow_Tools::Stream_MediaMajorType2StringMap.end ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media majortype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.majortype).c_str ())));
    result +=
      Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.majortype);
  } // end IF
  else
    result += (*iterator).second;
  result += ACE_TEXT_ALWAYS_CHAR ("\"\nsubtype: \"");
  result +=
    Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaType_in.subtype,
                                                       false);

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
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.formattype).c_str ())));
    result += Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.formattype);
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
  else if (mediaType_in.formattype == FORMAT_WaveFormatEx)
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
    result += ACE_TEXT_ALWAYS_CHAR ("---\nwFormatTag: \"");
    WORD2STRING_MAP_ITERATOR_T iterator =
      Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.find (waveformatex_p->wFormatTag);
    if (iterator == Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatType2StringMap.end ())
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
      if (Stream_Module_Device_Tools::isCompressedAudio (mediaType_in.subtype,
                                                         false))
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
      GUID2STRING_MAP_ITERATOR_T iterator =
        Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.find (waveformatextensible_p->SubFormat);
      if (iterator == Stream_Module_Device_DirectShow_Tools::Stream_WaveFormatSubType2StringMap.end ())
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("invalid/unknown wave subformat (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
        result += Stream_Module_Decoder_Tools::GUIDToString (waveformatextensible_p->SubFormat);
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
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_in.formattype).c_str ())));

  return result;
}
