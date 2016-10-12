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

#include <ace/Log_Msg.h>
#include <ace/Synch.h>

#include <initguid.h>
#include <strsafe.h>

#include "class_factory.h"
#include "registry.h"       // Helpers to register COM objects.

#include "common_time_common.h"

#include "stream_macros.h"

#include "stream_misc_common.h"
#include "stream_misc_mediafoundation_mediasource.h"

//#include "MPEG1ByteStreamHandler.h"

#include "test_i_target_common.h"

HMODULE g_hModule; // DLL module handle

// Defines the class factory lock variable:
DEFINE_CLASSFACTORY_SERVER_LOCK;

//#define IF_FAILED_GOTO (result, label) if (FAILED (result)) { goto label; }
//#define CHECK_HR (result) IF_FAILED_GOTO (result, done)

// -----------------------------------------------------------------------------

typedef Stream_Misc_MediaFoundation_MediaSource_T<Common_TimePolicy_t,
                                                  Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                                  Test_I_Target_MediaFoundation_Stream_Message,
                                                  Test_I_MediaFoundationConfiguration,
                                                  IMFMediaType> Stream_Misc_MediaFoundation_MediaSource_t;

// g_ClassFactories: Array of class factory data.
// Defines a look-up table of CLSIDs and corresponding creation functions.
ClassFactoryData g_ClassFactories[] =
{
  {&CLSID_ACEStream_MF_MediaSource, Stream_Misc_MediaFoundation_MediaSource_t::CreateInstance }
};
DWORD g_numClassFactories = ARRAYSIZE (g_ClassFactories);

// Text strings

// Description string for the bytestream handler.
//const TCHAR* sByteStreamHandlerDescription =
//  TEXT ("ACEStream Source ByteStreamHandler");
// File extension for WAVE files.
//const TCHAR* sFileExtension = TEXT(".mpg");
// Registry location for bytestream handlers.
//const TCHAR* REGKEY_MF_BYTESTREAM_HANDLERS =
//  TEXT ("Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers");

// Forward declarations
// Functions to register and unregister the byte stream handler.
HRESULT RegisterByteStreamHandler (const GUID& guid,
                                   const TCHAR* sFileExtension,
                                   const TCHAR* sDescription);
HRESULT UnregisterByteStreamHandler (const GUID& guid,
                                     const TCHAR* sFileExtension);

// Misc Registry helpers
HRESULT SetKeyValue (HKEY hKey, const TCHAR *sName, const TCHAR *sValue);

BOOL
APIENTRY DllMain (HANDLE hModule, 
                  DWORD  ul_reason_for_call, 
                  LPVOID lpReserved)
{
  STREAM_TRACE (ACE_TEXT ("::DllMain"));

  ACE_UNUSED_ARG (lpReserved);

  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
      g_hModule = (HMODULE)hModule;
      //TRACE_INIT();
      break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      //TRACE_CLOSE();
      break;
  } // end SWITCH

  return TRUE;
}

STDAPI DllCanUnloadNow ()
{
  STREAM_TRACE (ACE_TEXT ("::DllCanUnloadNow"));

  return (!ClassFactory::IsLocked () ? S_OK : S_FALSE);
}

STDAPI DllRegisterServer ()
{
  STREAM_TRACE (ACE_TEXT ("::DllRegisterServer"));

  HRESULT result = S_OK;

  // Register the bytestream handler's CLSID as a COM object.
  result = RegisterObject (g_hModule,
                           CLSID_ACEStream_MF_MediaSource,
                           ACE_TEXT_ALWAYS_CHAR (MODULE_MISC_MF_WIN32_BYTESTREAMHANDLER_DESCRIPTION),
                           ACE_TEXT_ALWAYS_CHAR ("Both"));
  if (FAILED (result)) goto done;

  //// Register the bytestream handler as the handler for the MPEG-1 file extension.
  //result = RegisterByteStreamHandler (CLSID_MFSampleMPEG1ByteStreamHandler, // CLSID 
  //                                    sFileExtension,                       // Supported file extension
  //                                    sByteStreamHandlerDescription);       // Description

done:
  return result;
}
STDAPI DllUnregisterServer ()
{
  STREAM_TRACE (ACE_TEXT ("::DllUnregisterServer"));

  // Unregister the CLSIDs
  UnregisterObject (CLSID_ACEStream_MF_MediaSource);

  //// Unregister the bytestream handler for the file extension.
  //UnregisterByteStreamHandler(CLSID_ACEStream_MF_MediaSource, sFileExtension);

  return S_OK;
}

STDAPI DllGetClassObject (REFCLSID CLSID_in,
                          REFIID riid,
                          void** ppv)
{
  STREAM_TRACE (ACE_TEXT ("::DllGetClassObject"));

  ClassFactory* class_factory_p = NULL;
  HRESULT result = CLASS_E_CLASSNOTAVAILABLE; // Default to failure

  // Find an entry in our look-up table for the specified CLSID.
  for (DWORD index = 0; index < g_numClassFactories; index++)
    if (*g_ClassFactories[index].pclsid == CLSID_in)
    {
      // Found an entry. Create a new class factory object.
      ACE_NEW_NORETURN (class_factory_p,
                        ClassFactory (g_ClassFactories[index].pfnCreate));
      result = (class_factory_p ? S_OK : E_OUTOFMEMORY);
      break;
    } // end IF
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ClassFactory(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF
  result = class_factory_p->QueryInterface (riid, ppv);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ClassFactory::QueryInterface(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    class_factory_p->Release ();

    return result;
  } // end IF
  class_factory_p->Release ();

  return result;
}

///////////////////////////////////////////////////////////////////////
// Name: CreateRegistryKey
// Desc: Creates a new registry key. (Thin wrapper just to encapsulate
//       all of the default options.)
///////////////////////////////////////////////////////////////////////
HRESULT
CreateRegistryKey (HKEY hKey,
                   LPCTSTR subkey,
                   HKEY *phKey)
{
  STREAM_TRACE (ACE_TEXT ("::CreateRegistryKey"));

  // sanity check(s)
  ACE_ASSERT (phKey);

  LONG result =
    RegCreateKeyEx (hKey,                 // parent key
                    subkey,               // name of subkey
                    0,                    // reserved
                    NULL,                 // class string (can be NULL)
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,                 // security attributes
                    phKey,
                    NULL);                // receives the "disposition" (is it a new or existing key)

  return HRESULT_FROM_WIN32 (result);
}

///////////////////////////////////////////////////////////////////////
// Name: RegisterByteStreamHandler
// Desc: Register a bytestream handler for the Media Foundation
//       source resolver.
//
// guid:            CLSID of the bytestream handler.
// sFileExtension:  File extension.
// sDescription:    Description.
//
// Note: sFileExtension can also be a MIME type although that is not
//       illustrated in this sample.
///////////////////////////////////////////////////////////////////////
HRESULT
RegisterByteStreamHandler (const GUID& guid,
                           const TCHAR* sFileExtension,
                           const TCHAR* sDescription)
{
  STREAM_TRACE (ACE_TEXT ("::RegisterByteStreamHandler"));

  HRESULT result = S_OK;
  int result_2 = -1;

  // Open HKCU/<byte stream handlers>/<file extension>
    
  // Create {clsid} = <description> key

  HKEY    hKey = NULL;
  HKEY    hSubKey = NULL;

  OLECHAR szCLSID[CHARS_IN_GUID];
  size_t  cchDescription = 0;
  result = StringCchLength (sDescription, STRSAFE_MAX_CCH, &cchDescription);
  if (SUCCEEDED (result))
    result_2 = StringFromGUID2 (guid, szCLSID, CHARS_IN_GUID);
  if (result_2 == CHARS_IN_GUID + 1)
    result = CreateRegistryKey (HKEY_LOCAL_MACHINE,
                                ACE_TEXT_ALWAYS_CHAR (MODULE_MISC_MF_WIN32_REG_BYTESTREAMHANDLERS_KEY),
                                &hKey);
  if (SUCCEEDED (result))
    result = CreateRegistryKey (hKey,
                                sFileExtension,
                                &hSubKey);
  if (SUCCEEDED (result))
    result =
      RegSetValueEx (hSubKey,
                     ACE_TEXT_WCHAR_TO_TCHAR (szCLSID),
                     0,
                     REG_SZ,
                     (BYTE*)sDescription,
                     static_cast<DWORD> ((cchDescription + 1) * sizeof (TCHAR)));
  if (hSubKey)
    RegCloseKey (hSubKey);
  if (hKey)
    RegCloseKey (hKey);

  return result;
}
HRESULT
UnregisterByteStreamHandler (const GUID& guid,
                             const TCHAR* sFileExtension)
{
  STREAM_TRACE (ACE_TEXT ("::UnregisterByteStreamHandler"));

  TCHAR szKey[MAX_PATH];
  OLECHAR szCLSID[CHARS_IN_GUID];

  DWORD result = 0;
  HRESULT result_2 = S_OK;

  // Create the subkey name.
  result_2 = StringCchPrintf (szKey,
                              MAX_PATH,
                              TEXT ("%s\\%s"),
                              ACE_TEXT (MODULE_MISC_MF_WIN32_REG_BYTESTREAMHANDLERS_KEY),
                              sFileExtension);
  if (FAILED (result_2)) goto done;

  // Create the CLSID name in canonical form.
  result = StringFromGUID2 (guid,
                            szCLSID, CHARS_IN_GUID);
  if (result != CHARS_IN_GUID + 1) goto done;

  // Delete the CLSID entry under the subkey. 
  // Note: There might be multiple entries for this file extension, so we should not delete 
  // the entire subkey, just the entry for this CLSID.
  result = RegDeleteKeyValue (HKEY_LOCAL_MACHINE,
                              szKey,
                              ACE_TEXT_WCHAR_TO_TCHAR (szCLSID));
  if (result != ERROR_SUCCESS)
  {
    result_2 = HRESULT_FROM_WIN32 (result);
    if (FAILED (result_2)) goto done;
  } // end IF

done:
  return result_2;
}
