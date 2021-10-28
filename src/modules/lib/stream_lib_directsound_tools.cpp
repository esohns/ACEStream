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

#include "initguid.h"
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

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

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
