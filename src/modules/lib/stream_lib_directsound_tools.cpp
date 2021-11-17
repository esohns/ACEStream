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

#define INITGUID
#include "guiddef.h"
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

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"

#include "stream_lib_directshow_tools.h"

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
  if (directsound_device_description_p->WaveDeviceId == cbdata_p->deviceId)
  {
    delete [] directsound_device_description_p; directsound_device_description_p = NULL;
    cbdata_p->deviceGUID = *lpGuid;
    return FALSE; // done
  } // end IF
  delete [] directsound_device_description_p; directsound_device_description_p = NULL;

  return TRUE;
}

struct _GUID
Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (ULONG waveDeviceId_in)
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
  result =
    DirectSoundCaptureEnumerate (stream_directshow_device_enumeration_a_cb, &cb_data_s);
  ACE_ASSERT (!FAILED (result));
  cb_data_s.IPropertySet->Release (); cb_data_s.IPropertySet = NULL;
  FreeLibrary (library_h);

  return cb_data_s.deviceGUID;
}

IAudioVolumeLevel*
Stream_MediaFramework_DirectSound_Tools::getMicrophoneBoostControl (IMMDevice* device_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::getMicrophoneBoostControl"));

  IDeviceTopology* topology_p = NULL;
  HRESULT result_2 =
    device_in->Activate (__uuidof (IDeviceTopology), CLSCTX_ALL, NULL,
                         (void**)&topology_p);
  ACE_ASSERT (SUCCEEDED (result_2));
  IConnector* connector_p = NULL;
  result_2 = topology_p->GetConnector (0, &connector_p);
  ACE_ASSERT (SUCCEEDED (result_2));
  topology_p->Release (); topology_p = NULL;
  IConnector* connector_2 = NULL;
  result_2 = connector_p->GetConnectedTo (&connector_2);
  ACE_ASSERT (SUCCEEDED (result_2));
  connector_p->Release (); connector_p = NULL;
  IPart* part_p = NULL;
  result_2 = connector_2->QueryInterface (IID_PPV_ARGS (&part_p));
  ACE_ASSERT (SUCCEEDED (result_2));
  connector_2->Release (); connector_2 = NULL;
  
  return Stream_MediaFramework_DirectSound_Tools::walkDeviceTreeFromPart (part_p,
                                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSOUND_MIC_BOOST_PART_DEFAULT_NAME));
}

void
Stream_MediaFramework_DirectSound_Tools::getAudioRendererStatistics (IFilterGraph* graph_in,
                                                                     Stream_MediaFrameWork_DirectSound_Statistics_t& statistic_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::getAudioRendererStatistics"));

  // sanity check(s)
  ACE_ASSERT (graph_in);

  // initialize return value(s)
  statistic_out.clear ();

  // step1: retrieve filter
  IBaseFilter* filter_p = NULL;
  HRESULT result =
    graph_in->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO,
                                &filter_p);
  ACE_ASSERT (SUCCEEDED (result) && filter_p);

  // step2: retrieve interface
  IAMAudioRendererStats* statistic_p = NULL;
  result = filter_p->QueryInterface (IID_PPV_ARGS (&statistic_p));
  if (unlikely (FAILED (result) || !statistic_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IAMAudioRendererStats): \"%s\", returning\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    filter_p->Release (); filter_p = NULL;
    return;
  } // end IF
  filter_p->Release (); filter_p = NULL;

  // step3: retrieve information
  DWORD value_1 = 0, value_2 = 0;
  for (DWORD i = AM_AUDREND_STAT_PARAM_BREAK_COUNT;
       i <= AM_AUDREND_STAT_PARAM_JITTER;
       ++i)
  {
    result = statistic_p->GetStatParam (i,
                                        &value_1,
                                        &value_2);
    if (unlikely (FAILED (result))) // 6: AM_AUDREND_STAT_PARAM_SLAVE_RATE: "...Valid only when the
                                    // DirectSound Renderer is matching rates to another clock or a live source. ..."
      continue;
    statistic_out[static_cast<enum _AM_AUDIO_RENDERER_STAT_PARAM> (i)] = std::make_pair (value_1, value_2);
  } // end FOR
  statistic_p->Release ();
}

std::string
Stream_MediaFramework_DirectSound_Tools::toString (enum _AM_AUDIO_RENDERER_STAT_PARAM parameter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectSound_Tools::toString"));

  switch (parameter_in)
  {
    case AM_AUDREND_STAT_PARAM_BREAK_COUNT:
      return ACE_TEXT_ALWAYS_CHAR ("BREAK_COUNT");
    case AM_AUDREND_STAT_PARAM_SLAVE_MODE:
      return ACE_TEXT_ALWAYS_CHAR ("SLAVE_MODE");
    case AM_AUDREND_STAT_PARAM_SILENCE_DUR:
      return ACE_TEXT_ALWAYS_CHAR ("SILENCE_DUR");
    case AM_AUDREND_STAT_PARAM_LAST_BUFFER_DUR:
      return ACE_TEXT_ALWAYS_CHAR ("LAST_BUFFER_DUR");
    case AM_AUDREND_STAT_PARAM_DISCONTINUITIES:
      return ACE_TEXT_ALWAYS_CHAR ("DISCONTINUITIES");
    case AM_AUDREND_STAT_PARAM_SLAVE_RATE:
      return ACE_TEXT_ALWAYS_CHAR ("SLAVE_RATE");
    case AM_AUDREND_STAT_PARAM_SLAVE_DROPWRITE_DUR:
      return ACE_TEXT_ALWAYS_CHAR ("DROPWRITE_DUR");
    case AM_AUDREND_STAT_PARAM_SLAVE_HIGHLOWERROR:
      return ACE_TEXT_ALWAYS_CHAR ("HIGHLOWERROR");
    case AM_AUDREND_STAT_PARAM_SLAVE_LASTHIGHLOWERROR:
      return ACE_TEXT_ALWAYS_CHAR ("LASTHIGHLOWERROR");
    case AM_AUDREND_STAT_PARAM_SLAVE_ACCUMERROR:
      return ACE_TEXT_ALWAYS_CHAR ("SLAVE_ACCUMERROR");
    case AM_AUDREND_STAT_PARAM_BUFFERFULLNESS:
      return ACE_TEXT_ALWAYS_CHAR ("BUFFERFULLNESS");
    case AM_AUDREND_STAT_PARAM_JITTER:
      return ACE_TEXT_ALWAYS_CHAR ("JITTER");
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown parameter (was: %d), aborting\n"),
                  parameter_in));
      break;
    }
  } // end SWITCH

  return ACE_TEXT_ALWAYS_CHAR ("");
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
