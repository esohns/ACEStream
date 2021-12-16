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

#include "stream_lib_directsound_tools.h"

#include "Audioclient.h"
#define INITGUID
#include "guiddef.h"
#include "devicetopology.h"
#include "endpointvolume.h"
#include "mmeapi.h"
#include "mmreg.h"
// *WARNING*: "...Note Header files ksproxy.h and dsound.h define similar but
//            incompatible versions of the IKsPropertySet interface.Applications
//            that require the KS proxy module should use the version defined in
//            ksproxy.h.The DirectSound version of IKsPropertySet is described
//            in the DirectSound reference pages in the Microsoft Windows SDK
//            documentation.
//            If an application must include both ksproxy.h and dsound.h,
//            whichever header file the compiler scans first is the one whose
//            definition of IKsPropertySet is used by the compiler. ..."
#include "dsound.h"
#include "dsconf.h"
#include "ks.h"
#include "strmif.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"

#include "stream_lib_directshow_tools.h"

// initialize statics
Stream_MediaFramework_DirectSound_Tools::WORD_TO_STRING_MAP_T Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap;
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap;

struct stream_directshow_device_enumeration_cbdata
{
  struct _GUID    deviceGUID;
  ULONG           deviceId;
  IKsPropertySet* IPropertySet;
};
BOOL CALLBACK
stream_directshow_device_enumeration_a_cb (LPGUID lpGuid,
                                           LPCSTR lpcstrDescription,
                                           LPCSTR lpcstrModule,
                                           LPVOID lpContext)
{
  // sanity check(s)
  struct stream_directshow_device_enumeration_cbdata* cbdata_p =
    static_cast<struct stream_directshow_device_enumeration_cbdata*> (lpContext);
  ACE_ASSERT (cbdata_p);
  ACE_ASSERT (cbdata_p->IPropertySet);
  if (!lpGuid)
    return TRUE;

  DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA directsound_device_description_s;
  ACE_OS::memset (&directsound_device_description_s, 0, sizeof (DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA));
  directsound_device_description_s.DeviceId = *lpGuid;
  DWORD cb_returned_i = 0;
  HRESULT result =
    cbdata_p->IPropertySet->Get (DSPROPSETID_DirectSoundDevice,
                                 DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
                                 NULL, 0,
                                 &directsound_device_description_s,
                                 sizeof (DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA),
                                 &cb_returned_i);
  ACE_ASSERT (SUCCEEDED (result));
  PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA directsound_device_description_p =
    (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA)new BYTE[cb_returned_i];
  ACE_ASSERT (directsound_device_description_p);
  *directsound_device_description_p = directsound_device_description_s;
  result =
    cbdata_p->IPropertySet->Get (DSPROPSETID_DirectSoundDevice,
                                 DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
                                 NULL, 0,
                                 directsound_device_description_p,
                                 cb_returned_i,
                                 &cb_returned_i);
  ACE_ASSERT (SUCCEEDED (result));
  if ((cbdata_p->deviceId != std::numeric_limits<ULONG>::max ()) &&
      (directsound_device_description_p->WaveDeviceId == cbdata_p->deviceId))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("found device (id: %u): \"%s\"; GUID: \"%s\"\n"),
                cbdata_p->deviceId,
                ACE_TEXT (directsound_device_description_p->Description),
                ACE_TEXT (Common_Tools::GUIDToString (*lpGuid).c_str ())));
    cbdata_p->deviceGUID = *lpGuid;
    delete [] directsound_device_description_p; directsound_device_description_p = NULL;
    return FALSE; // done
  } // end IF
  else if (!InlineIsEqualGUID (cbdata_p->deviceGUID, GUID_NULL) &&
           InlineIsEqualGUID (cbdata_p->deviceGUID, *lpGuid))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("found device (GUID: \"%s\"): \"%s\"; id: %u\n"),
                ACE_TEXT (Common_Tools::GUIDToString (*lpGuid).c_str ()),
                ACE_TEXT (directsound_device_description_p->Description),
                directsound_device_description_p->WaveDeviceId));
    cbdata_p->deviceId = directsound_device_description_p->WaveDeviceId;
    delete[] directsound_device_description_p; directsound_device_description_p = NULL;
    return FALSE; // done
  } // end ELSE IF
  delete [] directsound_device_description_p; directsound_device_description_p = NULL;

  return TRUE;
}

void
Stream_MediaFramework_DirectSound_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::initialize"));

  // ---------------------------------------------------------------------------

  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNKNOWN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VSELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IBM_CVSD, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ALAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MULAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DTS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DRM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAVOICE9, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAVOICE10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OKI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DVI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IMA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MEDIASPACE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIERRA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G723_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGISTD, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGIFIX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIALOGIC_OKI_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MEDIAVISION_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CU_CODEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_HP_DYN_VOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_YAMAHA_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONARC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DSPGROUP_TRUESPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ECHOSC1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AUDIOFILE_AF36, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_APTX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AUDIOFILE_AF10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PROSODY_1612, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LRC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DOLBY_AC2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSNAUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ANTEX_ADPCME, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONTROL_RES_VQLPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGIREAL, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGIADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONTROL_RES_CR10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NMS_VBXADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CS_IMAADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ECHOSC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ROCKWELL_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ROCKWELL_DIGITALK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_XEBEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G721_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G728_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSG723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SHARP_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEGLAYER3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CIRRUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ESPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CANOPUS_ATRAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G726_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G722_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DSAT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DSAT_DISPLAY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_BYTE_ALIGNED, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC16, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_AC20, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT29, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT29HW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_VR12, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_VR18, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_TQ40, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_SC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_SC3_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SOFTSOUND, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_TQ60, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSRT24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G729A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MVI_MVI2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DF_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DF_GSM610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ISIAUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ONLIVE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MULTITUDE_FT_SX20, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INFOCOM_ITS_G721_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONVEDIA_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CONGRUENCY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SBC24, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DOLBY_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MEDIASONIC_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PROSODY_8KBPS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ZYXEL_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_LPCBB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PACKED, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MALDEN_PHONYTALK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_GSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_G720_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RACAL_RECORDER_TETRA_ACELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NEC_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RAW_AAC1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RHETOREX_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IRAT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VIVO_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VIVO_SIREN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_PHILIPS_GRUNDIG, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIGITAL_G723, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SANYO_LD_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACEPLNET, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACELP4800, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_ACELP8V3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_G729A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SIPROLAB_KELVIN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOICEAGE_AMR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_G726ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DICTAPHONE_CELP68, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DICTAPHONE_CELP54, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QUALCOMM_PUREVOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QUALCOMM_HALFRATE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TUBGSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MSAUDIO1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMAUDIO_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMASPDIF, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ULAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_ALAW, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UNISYS_NAP_16K, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC008, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_G726L, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_CELP54, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYCOM_ACM_SYC701_CELP68, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_KNOWLEDGE_ADVENTURE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FRAUNHOFER_IIS_MPEG2_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DTS_DS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_FASTSPEECH8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CREATIVE_FASTSPEECH10, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_UHER_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ULEAD_DV_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ULEAD_DV_AUDIO_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QUARTERDECK, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ILINK_VC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RAW_SPORT, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ESST_AC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GENERIC_PASSTHRU, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IPI_HSX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_IPI_RPELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CS2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SCX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SCY, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_ATRAC3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONY_SPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TELUM_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TELUM_IA_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NORCOM_VOICE_SYSTEMS_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FM_TOWNS_SND, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MICRONAS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MICRONAS_CELP833, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_BTV_DIGITAL, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INTEL_MUSIC_CODER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INDEO_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_QDESIGN_MUSIC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ON2_VP7_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ON2_VP6_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VME_VMPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_TPC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LIGHTWAVE_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLIGSM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLIADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLICELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLISBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OLIOPR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_CELP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC8, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC12, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LH_CODEC_SBC16, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NORRIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ISIAUDIO_2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_LOAS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VODAFONE_MPEG_ADTS_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VODAFONE_MPEG_RAW_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOXWARE_RT24_SPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SONICFOUNDRY_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INNINGS_TELECOM_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_SX8300P, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LUCENT_SX5363S, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CUSEEME, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NTCSOFT_ALF2CM_ACM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DVM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DTS2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MAKEAVIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIVIO_MPEG4_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NOKIA_ADAPTIVE_MULTIRATE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DIVIO_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LEAD_SPEECH, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_LEAD_VORBIS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WAVPACK_AUDIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_2, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_3, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_1_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_2_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_OGG_VORBIS_MODE_3_PLUS, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_3COM_NBX, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FAAD_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AMR_NB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_AMR_WP, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_AMR_CBR, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_AMR_VBR_SID, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_AVQSBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_COMVERSE_INFOSYS_SBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SYMBOL_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOICEAGE_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_INGENIENT_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_MPEG4_AAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ENCORE_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_ZOLL_ASAO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_SPEEX_VOICE, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VIANIX_MASC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WM9_SPECTRUM_ANALYZER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_WMF_SPECTRUM_ANAYZER, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_610, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_620, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_660, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_690, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GSM_ADAPTIVE_MULTIRATE_WB, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G722, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_POLYCOM_SIREN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_GLOBAL_IP_ILBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_RADIOTIME_TIME_SHIFT_RADIO, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NICE_ACA, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NICE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G721, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G726, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G722_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G729_A, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_G723_1, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_VOCORD_LBC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_NICE_G728, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FRACE_TELECOM_G729, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_CODIAN, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_FLAC, ACE_TEXT_ALWAYS_CHAR ("Unknown")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_EXTENSIBLE, ACE_TEXT_ALWAYS_CHAR ("EXTENSIBLE")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.insert (std::make_pair (WAVE_FORMAT_DEVELOPMENT, ACE_TEXT_ALWAYS_CHAR ("DEVELOPMENT")));

  // ---------------------------------------------------------------------------

  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ANALOG, ACE_TEXT_ALWAYS_CHAR ("Analog")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_DRM, ACE_TEXT_ALWAYS_CHAR ("DRM")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ALAW, ACE_TEXT_ALWAYS_CHAR ("ALAW")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MULAW, ACE_TEXT_ALWAYS_CHAR ("MULAW")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_ADPCM, ACE_TEXT_ALWAYS_CHAR ("ADPCM")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEG, ACE_TEXT_ALWAYS_CHAR ("MPEG")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL, ACE_TEXT_ALWAYS_CHAR ("DOLBY_DIGITAL")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_WMA_PRO, ACE_TEXT_ALWAYS_CHAR ("WMA_PRO")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DTS, ACE_TEXT_ALWAYS_CHAR ("DTS")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG1, ACE_TEXT_ALWAYS_CHAR ("MPEG1")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG2, ACE_TEXT_ALWAYS_CHAR ("MPEG2")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_MPEG3, ACE_TEXT_ALWAYS_CHAR ("MPEG3")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_AAC, ACE_TEXT_ALWAYS_CHAR ("AAC")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_ATRAC, ACE_TEXT_ALWAYS_CHAR ("ATRAC")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_ONE_BIT_AUDIO, ACE_TEXT_ALWAYS_CHAR ("ONE_BIT_AUDIO")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS, ACE_TEXT_ALWAYS_CHAR ("DOLBY_DIGITAL_PLUS")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD, ACE_TEXT_ALWAYS_CHAR ("DTS_HD")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP, ACE_TEXT_ALWAYS_CHAR ("DOLBY_MLP")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_IEC61937_DST, ACE_TEXT_ALWAYS_CHAR ("DST")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEGLAYER3, ACE_TEXT_ALWAYS_CHAR ("MPEGLAYER3")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_MPEG_HEAAC, ACE_TEXT_ALWAYS_CHAR ("HEAAC")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO2, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO2")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO3, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO3")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_WMAUDIO_LOSSLESS, ACE_TEXT_ALWAYS_CHAR ("WMAUDIO_LOSSLESS")));
#endif // _WIN32_WINNT && (_WIN32_WINNT > 0x0601) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_DTS_AUDIO, ACE_TEXT_ALWAYS_CHAR ("DTS_AUDIO")));
  Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.insert (std::make_pair (KSDATAFORMAT_SUBTYPE_SDDS_AUDIO, ACE_TEXT_ALWAYS_CHAR ("SDDS_AUDIO")));
}

struct _GUID
Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (ULONG waveDeviceId_in,
                                                                        bool capture_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID"));

  HRESULT result = E_FAIL;
  HMODULE library_h = LoadLibrary (ACE_TEXT ("dsound.dll"));
  ACE_ASSERT (library_h);
  LPFNGETCLASSOBJECT pDllGetClassObject =
    (LPFNGETCLASSOBJECT)GetProcAddress (library_h, ACE_TEXT_ALWAYS_CHAR ("DllGetClassObject"));
  ACE_ASSERT (pDllGetClassObject);
  LPCLASSFACTORY class_factory_p = NULL;
  result =
    pDllGetClassObject (CLSID_DirectSoundPrivate, IID_PPV_ARGS (&class_factory_p));
  ACE_ASSERT (!FAILED (result));
  //LPKSPROPERTYSET property_set_p = NULL;
  struct stream_directshow_device_enumeration_cbdata cb_data_s;
  cb_data_s.deviceGUID = GUID_NULL;
  cb_data_s.deviceId = waveDeviceId_in;
  result =
    class_factory_p->CreateInstance (NULL, IID_IKsPropertySet, (void**)(&cb_data_s.IPropertySet));
  ACE_ASSERT (!FAILED (result));
  class_factory_p->Release (); class_factory_p = NULL;
  if (capture_in)
    result =
      DirectSoundCaptureEnumerate (stream_directshow_device_enumeration_a_cb, &cb_data_s);
  else
    result =
      DirectSoundEnumerate (stream_directshow_device_enumeration_a_cb, &cb_data_s);
  ACE_ASSERT (!FAILED (result));
  cb_data_s.IPropertySet->Release (); cb_data_s.IPropertySet = NULL;
  FreeLibrary (library_h);

  return cb_data_s.deviceGUID;
}

ULONG
Stream_MediaFramework_DirectSound_Tools::directSoundGUIDTowaveDeviceId (REFGUID GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID"));

  HRESULT result = E_FAIL;
  HMODULE library_h = LoadLibrary (ACE_TEXT ("dsound.dll"));
  ACE_ASSERT (library_h);
  LPFNGETCLASSOBJECT pDllGetClassObject =
    (LPFNGETCLASSOBJECT)GetProcAddress (library_h, ACE_TEXT_ALWAYS_CHAR ("DllGetClassObject"));
  ACE_ASSERT (pDllGetClassObject);
  LPCLASSFACTORY class_factory_p = NULL;
  result =
    pDllGetClassObject (CLSID_DirectSoundPrivate, IID_PPV_ARGS (&class_factory_p));
  ACE_ASSERT (!FAILED (result));
  //LPKSPROPERTYSET property_set_p = NULL;
  struct stream_directshow_device_enumeration_cbdata cb_data_s;
  cb_data_s.deviceGUID = GUID_in;
  cb_data_s.deviceId = std::numeric_limits<ULONG>::max ();
  result =
    class_factory_p->CreateInstance (NULL, IID_IKsPropertySet, (void**)(&cb_data_s.IPropertySet));
  ACE_ASSERT (!FAILED (result));
  class_factory_p->Release (); class_factory_p = NULL;
  result =
    DirectSoundCaptureEnumerate (stream_directshow_device_enumeration_a_cb, &cb_data_s);
  ACE_ASSERT (!FAILED (result));
  if (cb_data_s.deviceId != std::numeric_limits<ULONG>::max ())
  {
    cb_data_s.IPropertySet->Release (); cb_data_s.IPropertySet = NULL;
    FreeLibrary (library_h);
    return cb_data_s.deviceId;
  } // end IF
  result =
    DirectSoundEnumerate (stream_directshow_device_enumeration_a_cb, &cb_data_s);
  ACE_ASSERT (!FAILED (result));
  cb_data_s.IPropertySet->Release (); cb_data_s.IPropertySet = NULL;
  FreeLibrary (library_h);

  return cb_data_s.deviceId;
}

IAudioEndpointVolume*
Stream_MediaFramework_DirectSound_Tools::getVolumeControl (REFGUID deviceIdentifier_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::getVolumeControl"));

  // initialize return value(s)
  IAudioEndpointVolume* result_p = NULL;

  IMMDevice* device_p =
    Stream_MediaFramework_DirectSound_Tools::getDevice (deviceIdentifier_in);
  if (unlikely (!device_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve device handle (id was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
    return NULL;
  } // end IF

  HRESULT result =
    device_p->Activate (__uuidof (IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL,
                        (LPVOID*)&result_p);
  ACE_ASSERT (SUCCEEDED (result) && result_p);
  device_p->Release (); device_p = NULL;

  return result_p;
}

IAudioVolumeLevel*
Stream_MediaFramework_DirectSound_Tools::getMicrophoneBoostControl (REFGUID deviceIdentifier_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::getMicrophoneBoostControl"));

  IMMDevice* device_p =
    Stream_MediaFramework_DirectSound_Tools::getDevice (deviceIdentifier_in);
  if (unlikely (!device_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve device handle (id was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
    return NULL;
  } // end IF

  IDeviceTopology* topology_p = NULL;
  HRESULT result_2 =
    device_p->Activate (__uuidof (IDeviceTopology), CLSCTX_ALL, NULL,
                        (void**)&topology_p);
  ACE_ASSERT (SUCCEEDED (result_2) && topology_p);
  device_p->Release (); device_p = NULL;
  IConnector* connector_p = NULL;
  result_2 = topology_p->GetConnector (0, &connector_p);
  ACE_ASSERT (SUCCEEDED (result_2) && connector_p);
  topology_p->Release (); topology_p = NULL;
  IConnector* connector_2 = NULL;
  result_2 = connector_p->GetConnectedTo (&connector_2);
  ACE_ASSERT (SUCCEEDED (result_2) && connector_2);
  connector_p->Release (); connector_p = NULL;
  IPart* part_p = NULL;
  result_2 = connector_2->QueryInterface (IID_PPV_ARGS (&part_p));
  ACE_ASSERT (SUCCEEDED (result_2) && part_p);
  connector_2->Release (); connector_2 = NULL;
  
  return Stream_MediaFramework_DirectSound_Tools::walkDeviceTreeFromPart (part_p,
                                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSOUND_MIC_BOOST_PART_DEFAULT_NAME));
}

bool
Stream_MediaFramework_DirectSound_Tools::canRender (ULONG deviceId_in,
                                                    const struct tWAVEFORMATEX& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::canRender"));

  WAVEOUTCAPS capabilities_s;
  ACE_OS::memset (&capabilities_s, 0, sizeof (WAVEOUTCAPS));
  MMRESULT result = waveOutGetDevCaps (deviceId_in,
                                       &capabilities_s,
                                       sizeof (WAVEOUTCAPS));
  if (unlikely (result != MMSYSERR_NOERROR))
  {
    char error_msg_a[BUFSIZ];
    waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waveOutGetDevCaps(%d): \"%s\", aborting\n"),
                deviceId_in, ACE_TEXT (error_msg_a)));
    return false; // *TODO*: false negative !
  } // end IF

  // check in turn:
  // - sample rate
  // - channels
  // - bits/sample
  // - value format

  switch (format_in.nSamplesPerSec)
  {
    case 11025:
    {
      switch (format_in.nChannels)
      {
        case 1:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_1M08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_1M16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        case 2:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_1S08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_1S16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown number of channels (was: %u), aborting\n"),
                      format_in.nChannels));
          return false; // *TODO*: false negative !
        }
      } // end SWITCH
      break;
    }
    case 22050:
    {
      switch (format_in.nChannels)
      {
        case 1:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_2M08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_2M16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        case 2:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_2S08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_2S16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown number of channels (was: %u), aborting\n"),
                      format_in.nChannels));
          return false; // *TODO*: false negative !
        }
      } // end SWITCH
      break;
    }
    case 44100:
    {
      switch (format_in.nChannels)
      {
        case 1:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_4M08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_4M16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        case 2:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_4S08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_4S16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown number of channels (was: %u), aborting\n"),
                      format_in.nChannels));
          return false; // *TODO*: false negative !
        }
      } // end SWITCH
      break;
    }
    case 48000:
    {
      switch (format_in.nChannels)
      {
        case 1:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_48M08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_48M16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        case 2:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_48S08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_48S16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown number of channels (was: %u), aborting\n"),
                      format_in.nChannels));
          return false; // *TODO*: false negative !
        }
      } // end SWITCH
      break;
    }
    case 96000:
    {
      switch (format_in.nChannels)
      {
        case 1:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_96M08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_96M16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        case 2:
        {
          switch (format_in.wBitsPerSample)
          {
            case 8:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_96S08))
                return false;
              break;
            case 16:
              if (!(capabilities_s.dwFormats & WAVE_FORMAT_96S16))
                return false;
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("invalid/unknown bits per sample (was: %u), aborting\n"),
                          format_in.wBitsPerSample));
              return false; // *TODO*: false negative !
            }
          } // end SWITCH
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown number of channels (was: %u), aborting\n"),
                      format_in.nChannels));
          return false; // *TODO*: false negative !
        }
      } // end SWITCH
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown sample rate (was: %u), aborting\n"),
                  format_in.nSamplesPerSec));
      return false; // *TODO*: false negative !
    }
  } // end SWITCH

  result = waveOutOpen (NULL,
                        deviceId_in,
                        &format_in,
                        NULL,
                        NULL,
                        WAVE_FORMAT_QUERY);
  if (result == MMSYSERR_NOERROR)
    return true;
  if (result == WAVERR_BADFORMAT)
    return false;
  char error_msg_a[BUFSIZ];
  waveOutGetErrorText (result, error_msg_a, BUFSIZ - 1);
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to waveOutGetDevCaps(%d): \"%s\", aborting\n"),
              deviceId_in, ACE_TEXT (error_msg_a)));
  return false; // *TODO*: false negative !
}

void
Stream_MediaFramework_DirectSound_Tools::getBestFormat (ULONG deviceId_in,
                                                        struct tWAVEFORMATEX& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::getBestFormat"));

  // initialize return value(s)
  ACE_OS::memset (&format_out, 0, sizeof (struct tWAVEFORMATEX));

  format_out.wFormatTag =
    STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_FORMAT;
  format_out.nChannels =
    STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_CHANNELS;
  format_out.nSamplesPerSec =
    STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_SAMPLES_PER_SECOND;
  format_out.wBitsPerSample =
    STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_BITS_PER_SAMPLE;
  format_out.nBlockAlign =
    (format_out.nChannels * (format_out.wBitsPerSample / 8));
  format_out.nAvgBytesPerSec =
    (format_out.nSamplesPerSec * format_out.nBlockAlign);

  if (Stream_MediaFramework_DirectSound_Tools::canRender (deviceId_in,
                                                          format_out))
    return;

  std::vector<WORD> formats_a = {WAVE_FORMAT_IEEE_FLOAT, WAVE_FORMAT_PCM};
  std::vector<WORD>::const_iterator iterator = formats_a.begin ();
  std::vector<WORD> channels_a = {2, 1};
  std::vector<WORD>::const_iterator iterator_2 = channels_a.begin ();
  std::vector<DWORD> rates_a = {96000, 48000, 44100, 22050, 11025, 8000, 4000};
  std::vector<DWORD>::const_iterator iterator_3 = rates_a.begin ();
  std::vector<DWORD> resolutions_a = {32, 16, 8};
  std::vector<DWORD>::const_iterator iterator_4 = resolutions_a.begin ();
  do
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%u: trying %s...\n"),
                deviceId_in,
                ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (format_out, true).c_str ())));

    format_out.wFormatTag = *iterator;
    format_out.nChannels = static_cast<WORD> (*iterator_2);
    format_out.nSamplesPerSec = *iterator_3;
    format_out.wBitsPerSample = static_cast<WORD> (*iterator_4);
    format_out.nBlockAlign =
      (format_out.nChannels * (format_out.wBitsPerSample / 8));
    format_out.nAvgBytesPerSec =
      (format_out.nSamplesPerSec * format_out.nBlockAlign);

    if (Stream_MediaFramework_DirectSound_Tools::canRender (deviceId_in,
                                                            format_out))
      return;

    // *NOTE*: the logic is this:
    // - prefer more channels
    // - (then) prefer higher rates
    // - (then) prefer higher resolutions
    // - (then) prefer IEEE float over PCM
    if ((*iterator) != WAVE_FORMAT_PCM)
    {
      ++iterator;
      continue;
    } // end IF
    iterator = formats_a.begin (); // reset
    if ((*iterator_4) != 8)
    {
      ++iterator_4;
      continue;
    } // end IF
    iterator_4 = resolutions_a.begin (); // reset
    if ((*iterator_3) != 4000)
    {
      ++iterator_3;
      continue;
    } // end IF
    iterator_3 = rates_a.begin (); // reset
    if ((*iterator_2) != 1)
    {
      ++iterator_2;
      continue;
    } // end IF
    iterator_2 = channels_a.begin (); // reset
  } while (true);
}

bool
Stream_MediaFramework_DirectSound_Tools::canRender (REFGUID deviceIdentifier_in,
                                                    enum _AUDCLNT_SHAREMODE shareMode_in,
                                                    const struct tWAVEFORMATEX& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::canRender"));

  IMMDevice* device_p =
    Stream_MediaFramework_DirectSound_Tools::getDevice (deviceIdentifier_in);
  if (unlikely (!device_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve device handle (id was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
    return false; // *TODO*: false negative !
  } // end IF

  IAudioClient* audio_client_p = NULL;
  HRESULT result = device_p->Activate (__uuidof (IAudioClient), CLSCTX_ALL,
                                       NULL, (void**)&audio_client_p);
  ACE_ASSERT (SUCCEEDED (result) && audio_client_p);
  device_p->Release (); device_p = NULL;
  struct tWAVEFORMATEX* audio_info_p = NULL;
  result = audio_client_p->IsFormatSupported (shareMode_in,
                                              &format_in,
                                              &audio_info_p);
  if (FAILED (result))
  { ACE_ASSERT (!audio_info_p);
    audio_client_p->Release (); audio_client_p = NULL;
    return false;
  } // end IF
  if (result == S_FALSE)
  {
    audio_client_p->Release (); audio_client_p = NULL;
    CoTaskMemFree (audio_info_p); audio_info_p = NULL;
    return false;
  } // end IF
  audio_client_p->Release (); audio_client_p = NULL;
  CoTaskMemFree (audio_info_p); audio_info_p = NULL;
  return true;
}

void
Stream_MediaFramework_DirectSound_Tools::getAudioEngineMixFormat (REFGUID deviceIdentifier_in,
                                                                  struct tWAVEFORMATEX& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::getAudioEngineMixFormat"));

  // initialize return value(s)
  ACE_OS::memset (&format_out, 0, sizeof (struct tWAVEFORMATEX));

  IMMDevice* device_p =
    Stream_MediaFramework_DirectSound_Tools::getDevice (deviceIdentifier_in);
  if (unlikely (!device_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve device handle (id was: \"%s\"), returning\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
    return;
  } // end IF

  IAudioClient* audio_client_p = NULL;
  HRESULT result = device_p->Activate (__uuidof (IAudioClient), CLSCTX_ALL,
                                       NULL, (void**)&audio_client_p);
  ACE_ASSERT (SUCCEEDED (result) && audio_client_p);
  device_p->Release (); device_p = NULL;
  struct tWAVEFORMATEX* audio_info_p = NULL;
  result = audio_client_p->GetMixFormat (&audio_info_p);
  ACE_ASSERT (SUCCEEDED (result) && audio_info_p);
  audio_client_p->Release (); audio_client_p = NULL;
  format_out = *audio_info_p;
  CoTaskMemFree (audio_info_p); audio_info_p = NULL;
}

IMMDevice*
Stream_MediaFramework_DirectSound_Tools::getDevice (REFGUID deviceIdentifier_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::getDevice"));

  // initialize return value(s)
  IMMDevice* result_p = NULL;

  // sanity check(s)
  IMMDeviceEnumerator* enumerator_p = NULL;
  HRESULT result =
    CoCreateInstance (__uuidof (MMDeviceEnumerator), NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&enumerator_p));
  ACE_ASSERT (SUCCEEDED (result) && enumerator_p);
  if (InlineIsEqualGUID (deviceIdentifier_in, GUID_NULL))
  {
    result =
      enumerator_p->GetDefaultAudioEndpoint (eRender, eConsole, &result_p);
    ACE_ASSERT (SUCCEEDED (result) && result_p);
    enumerator_p->Release (); enumerator_p = NULL;
    return result_p;
  } // end IF

  IMMDeviceCollection* devices_p = NULL;
  UINT num_devices_i = 0;
  struct _GUID GUID_s = GUID_NULL;
  IPropertyStore* property_store_p = NULL;
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);

  result = enumerator_p->EnumAudioEndpoints (eAll,
                                             DEVICE_STATEMASK_ALL,
                                             &devices_p);
  ACE_ASSERT (SUCCEEDED (result) && devices_p);
  enumerator_p->Release (); enumerator_p = NULL;
  result = devices_p->GetCount (&num_devices_i);
  ACE_ASSERT (SUCCEEDED (result));
  for (UINT i = 0;
       i < num_devices_i;
       ++i)
  { ACE_ASSERT (!result_p);
    result = devices_p->Item (i,
                              &result_p);
    ACE_ASSERT (SUCCEEDED (result) && result_p);
    result = result_p->OpenPropertyStore (STGM_READ,
                                          &property_store_p);
    ACE_ASSERT (SUCCEEDED (result) && property_store_p);
    result = property_store_p->GetValue (PKEY_AudioEndpoint_GUID,
                                         &property_s);
    ACE_ASSERT (SUCCEEDED (result));
    property_store_p->Release (); property_store_p = NULL;
    ACE_ASSERT (property_s.vt == VT_LPWSTR);
    GUID_s =
      Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (property_s.pwszVal)));
    PropVariantClear (&property_s);
    if (InlineIsEqualGUID (GUID_s, deviceIdentifier_in))
      break;
    result_p->Release (); result_p = NULL;
  } // end FOR
  devices_p->Release (); devices_p = NULL;
  if (!result_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve device handle (id was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
    return NULL;
  } // end IF

  return result_p;
}

IAudioVolumeLevel*
Stream_MediaFramework_DirectSound_Tools::walkDeviceTreeFromPart (IPart* part_in,
                                                                 const std::string& volumeControlName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::walkDeviceTreeFromPart"));

  IAudioVolumeLevel* result = NULL;

  LPWSTR pwszPartName = NULL;
  HRESULT result_2 = part_in->GetName (&pwszPartName);
  ACE_ASSERT (SUCCEEDED (result_2));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("found part \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (pwszPartName)));
  if (!pwszPartName ||
      (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (pwszPartName)) != volumeControlName_in))
  {
    CoTaskMemFree (pwszPartName); pwszPartName = NULL;
    goto continue_;
  } // end IF
  CoTaskMemFree (pwszPartName); pwszPartName = NULL;

  result_2 = part_in->Activate (CLSCTX_ALL,
                                IID_PPV_ARGS (&result));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("part \"%s\" is not a volume node, aborting\n"),
                ACE_TEXT (volumeControlName_in.c_str ())));
    part_in->Release ();
    return NULL;
  } // end IF
  part_in->Release ();

  return result;

continue_:
  IPartsList* parts_p = NULL;
  result_2 = part_in->EnumPartsOutgoing (&parts_p);
  if (FAILED (result_2))
  { // --> probably reached end of (sub-)path
    part_in->Release ();
    return NULL;
  } // end IF
  UINT num_parts_i = 0;
  result_2 = parts_p->GetCount (&num_parts_i);
  ACE_ASSERT (SUCCEEDED (result_2));
  IPart* part_p = NULL;
  PartType parttype_e;
  IConnector* connector_p = NULL, *connector_2 = NULL;
  BOOL connected_b = FALSE;
  for (UINT i = 0;
       i < num_parts_i;
       ++i)
  { ACE_ASSERT (!part_p);
    result_2 = parts_p->GetPart (i, &part_p);
    ACE_ASSERT (SUCCEEDED (result_2));
    result_2 = part_p->GetPartType (&parttype_e);
    ACE_ASSERT (SUCCEEDED (result_2));
    if (parttype_e == Connector)
    {
      result_2 = part_p->QueryInterface (IID_PPV_ARGS (&connector_p));
      ACE_ASSERT (SUCCEEDED (result_2));
      part_p->Release (); part_p = NULL;
      result_2 = connector_p->IsConnected (&connected_b);
      ACE_ASSERT (SUCCEEDED (result_2));
      if (!connected_b)
      {
        connector_p->Release (); connector_p = NULL;
        continue;
      } // end IF
      connector_p->GetConnectedTo (&connector_2);
      ACE_ASSERT (SUCCEEDED (result_2));
      connector_p->Release (); connector_p = NULL;
      result_2 = connector_2->QueryInterface (IID_PPV_ARGS (&part_p));
      ACE_ASSERT (SUCCEEDED (result_2));
      connector_2->Release (); connector_2 = NULL;
      result =
        Stream_MediaFramework_DirectSound_Tools::walkDeviceTreeFromPart (part_p,
                                                                         volumeControlName_in);
      part_p = NULL;
      if (result)
        break;
      continue;
    } // end IF

    result =
      Stream_MediaFramework_DirectSound_Tools::walkDeviceTreeFromPart (part_p,
                                                                       volumeControlName_in);
    part_p = NULL;
    if (result)
      break;
  } // end FOR
  parts_p->Release (); parts_p = NULL;
  part_in->Release ();

  return result;
}

std::string
Stream_MediaFramework_DirectSound_Tools::toString (const struct tWAVEFORMATEX& format_in,
                                                   bool condensed_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::toString"));

  if (condensed_in)
    return Stream_MediaFramework_DirectSound_Tools::toString_2 (format_in);

  std::string result;

  std::ostringstream converter;

  result += ACE_TEXT_ALWAYS_CHAR ("wFormatTag: \"");
  WORD_TO_STRING_MAP_ITERATOR_T iterator =
    Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.find (format_in.wFormatTag);
  if (iterator == Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown wave formattype (was: %d), continuing\n"),
                format_in.wFormatTag));
    converter << format_in.wFormatTag;
    result += converter.str ();
  } // end IF
  else
    result += (*iterator).second;

  result += ACE_TEXT_ALWAYS_CHAR ("\"\nnChannels: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nChannels;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\nnSamplesPerSec: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nSamplesPerSec;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\nnAvgBytesPerSec: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nAvgBytesPerSec;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\nnBlockAlign: "); // frame size
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nBlockAlign;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\nwBitsPerSample: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.wBitsPerSample;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("\ncbSize: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.cbSize;
  result += converter.str ();

  if (format_in.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
  {
    WAVEFORMATEXTENSIBLE* waveformatextensible_p =
      (WAVEFORMATEXTENSIBLE*)&format_in;

    result +=
      (!format_in.wBitsPerSample ? ACE_TEXT_ALWAYS_CHAR ("\nwSamplesPerBlock: ")
                                 : ACE_TEXT_ALWAYS_CHAR ("\nwValidBitsPerSample: "));
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatextensible_p->Samples.wValidBitsPerSample;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\ndwChannelMask: 0x");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter <<
      std::hex << waveformatextensible_p->dwChannelMask << std::dec;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR ("\nSubFormat: \"");
    Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator =
      Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.find (waveformatextensible_p->SubFormat);
    if (iterator == Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.end ())
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown wave subformat (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
      result += Common_Tools::GUIDToString (waveformatextensible_p->SubFormat);
    } // end IF
    else
      result += (*iterator).second;
    result += ACE_TEXT_ALWAYS_CHAR ("\"");
  } // end IF
  else // *TODO*
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("invalid/unknown wave formattype (was: %d), continuing\n"),
                format_in.wFormatTag));

  return result;
}

std::string
Stream_MediaFramework_DirectSound_Tools::toString_2 (const struct tWAVEFORMATEX& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::toString_2"));

  std::string result;

  std::ostringstream converter;

  result += ACE_TEXT_ALWAYS_CHAR ("\"");
  WORD_TO_STRING_MAP_ITERATOR_T iterator =
    Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.find (format_in.wFormatTag);
  if (iterator == Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatTypeToStringMap.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown wave formattype (was: %d), continuing\n"),
                format_in.wFormatTag));
    converter << format_in.wFormatTag;
    result += converter.str ();
  } // end IF
  else
    result += (*iterator).second;

  result += ACE_TEXT_ALWAYS_CHAR ("\" (#c|#s/s|avgb/s|ba|b/s): ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nChannels;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("|");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nSamplesPerSec;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("|");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nAvgBytesPerSec;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("|"); // frame size
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.nBlockAlign;
  result += converter.str ();

  result += ACE_TEXT_ALWAYS_CHAR ("|");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << format_in.wBitsPerSample;
  result += converter.str ();

  //result += ACE_TEXT_ALWAYS_CHAR ("\ncbSize: ");
  //converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  //converter.clear ();
  //converter << format_in.cbSize;
  //result += converter.str ();

  if (format_in.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
  {
    WAVEFORMATEXTENSIBLE* waveformatextensible_p =
      (WAVEFORMATEXTENSIBLE*)&format_in;

    result +=
      (!format_in.wBitsPerSample ? ACE_TEXT_ALWAYS_CHAR (" s/b: ")
                                 : ACE_TEXT_ALWAYS_CHAR (" vb/s: "));
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << waveformatextensible_p->Samples.wValidBitsPerSample;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR (" cm: 0x");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter <<
      std::hex << waveformatextensible_p->dwChannelMask << std::dec;
    result += converter.str ();

    result += ACE_TEXT_ALWAYS_CHAR (" sub-format: \"");
    Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator =
      Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.find (waveformatextensible_p->SubFormat);
    if (iterator == Stream_MediaFramework_DirectSound_Tools::Stream_WaveFormatSubTypeToStringMap.end ())
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown wave subformat (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
      result += Common_Tools::GUIDToString (waveformatextensible_p->SubFormat);
    } // end IF
    else
      result += (*iterator).second;
    result += ACE_TEXT_ALWAYS_CHAR ("\"");
  } // end IF

  return result;
}
