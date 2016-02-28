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

#include "stream_misc_directshow_source_filter.h"

//#include "combase.h"
//#include "dllsetup.h"
//#include "guiddef.h"
#include "initguid.h" // *NOTE*: this exports DEFINE_GUIDs
#include "streams.h"
//#include "strmif.h"
//#include "uuids.h"

#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"

#include "common_time_common.h"
#include "common_tools.h"

#include "class_factory.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "test_i_common_modules.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

// initialize static variables
static const WCHAR g_wszName[] = MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L;

// {F9F62434-535B-4934-A695-BE8D10A4C699}
DEFINE_GUID (CLSID_CamStream_Target_Source_Filter,
0xf9f62434, 0x535b, 0x4934, 0xa6, 0x95, 0xbe, 0x8d, 0x10, 0xa4, 0xc6, 0x99);

// Setup data
const AMOVIESETUP_MEDIATYPE sudMediaTypes[] =
{
    /*{ &MEDIATYPE_Video, &MEDIASUBTYPE_NULL  }
  ,*/ { &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 }
  //, { &MEDIATYPE_Video, &MEDIASUBTYPE_RGB32 }
  , { &MEDIATYPE_Video, &MEDIASUBTYPE_MJPG  }
  , { &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2  }
  ,
};
//const REGFILTERPINS2 sudOutputPin = 
//{
//  REG_PINFLAG_B_OUTPUT,  // dwFlags  (also _B_ZERO, _B_MANY, _B_RENDERER)
//  1,                     // cInstances
//  1,                     // nMediaTypes
//  &sudOpPinTypes,        // lpMediaType
//  0,                     // nMediums
//  NULL,                  // lpMedium
//  &PIN_CATEGORY_CAPTURE, // clsPinCategory
//};
//const REGFILTER2 sudFilterReg =
//{
//  2,                // Registration format. (2 --> REGFILTERPINS2)
//  MERIT_DO_NOT_USE, // Merit.
//  1,                // Number of pins
//  &sudOutputPin     // Pointer to pin information.
//};
//const AdvFilterInfo sudACEStreamax =
//{
//  &CLSID_ACEStream_Source_Filter,          // Filter CLSID
//  MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE, // String name
//  0,                                       // must be 0 for new structure
//  1,                                       // Number of pins
//  &sudOpPin,                               // Pin details
//  MERIT_DO_NOT_USE,                        // filter merit
//  &CLSID_VideoInputDeviceCategory,
//};
const AMOVIESETUP_PIN sudOutputPin =
{
  MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME, // Obsolete, not used.
  FALSE,                                       // rendered ?
  TRUE,                                        // output ?
  FALSE,                                       // Can the filter create zero instances ?
  FALSE,                                       // Does the filter create multiple instances ?
  &CLSID_NULL,                                 // Obsolete.
  NULL,                                        // Obsolete.
  5,                                           // Number of media types
  sudMediaTypes                                // Pointer to media types.
};
const AMOVIESETUP_FILTER sudFilterReg =
{
  &CLSID_CamStream_Target_Source_Filter,     // Filter CLSID.
  MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L, // Filter name.
  MERIT_DO_NOT_USE,                          // Merit.
  1,                                         // Number of pin types.
  &sudOutputPin                              // Pointer to pin information.
};

// -----------------------------------------------------------------------------

typedef Stream_Misc_DirectShow_Source_Filter_T<Common_TimePolicy_t,
                                               Test_I_Target_Stream_SessionMessage,
                                               Test_I_Target_Stream_Message,
                                               Test_I_Target_Stream_Module_DirectShowSource> Stream_Misc_DirectShow_Source_Filter_t;

CFactoryTemplate g_Templates[] = {
  { MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L                     // Name.
  , &CLSID_CamStream_Target_Source_Filter                         // CLSID.
  , Stream_Misc_DirectShow_Source_Filter_t::CreateInstance        // Creation function.
  //, Stream_Misc_DirectShow_Source_Filter_t::InitializeInstance
  , NULL                                                          // Initialization function.
  , &sudFilterReg }                                               // Pointer to filter information.
};
int g_cTemplates = sizeof (g_Templates) / sizeof (g_Templates[0]);

// -----------------------------------------------------------------------------

//STDAPI
//DllCanUnloadNow ()
//{
//  STREAM_TRACE (ACE_TEXT ("::DllCanUnloadNow"));
//
//  if (CClassFactory::IsLocked () ||
//      CBaseObject::ObjectsActive ())
//    return S_FALSE;
//
//  return S_OK;
//}
//
//STDAPI
//DllGetClassObject (__in REFCLSID rClsID_in,
//                   __in REFIID riid_in,
//                   __deref_out void** factory_out)
//{
//  STREAM_TRACE (ACE_TEXT ("::DllGetClassObject"));
//
//  // initialize return value(s)
//  *factory_out = NULL;
//
//  // sanity check(s)
//  if (!(riid_in == IID_IUnknown) &&
//      !(riid_in == IID_IClassFactory))
//    return E_NOINTERFACE;
//
//  const CFactoryTemplate* factory_template_p = NULL;
//  for (int i = 0; i < g_cTemplates; i++)
//  {
//    factory_template_p = &g_Templates[i];
//    if (factory_template_p->IsClassID (rClsID_in))
//      break;
//  } // end FOR
//  if (!factory_template_p)
//    return CLASS_E_CLASSNOTAVAILABLE;
//
//  ACE_NEW_NORETURN (*factory_out,
//                    CClassFactory (factory_template_p));
//  if (!*factory_out)
//    return E_OUTOFMEMORY;
//
//  ((LPUNKNOWN)*factory_out)->AddRef ();
//
//  return NOERROR;
//}

STDAPI
DllRegisterServer ()
{
  STREAM_TRACE (ACE_TEXT ("::DllRegisterServer"));

  HRESULT result = E_FAIL;

  result = AMovieDllRegisterServer2 (TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AMovieDllRegisterServer2(true): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

//  IFilterMapper2* ifilter_mapper_p = NULL;
//  result = CoCreateInstance (CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
//                             IID_IFilterMapper2, (void**)&ifilter_mapper_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterMapper2): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return result;
//  } // end IF
//  ACE_ASSERT (ifilter_mapper_p);
//
//  result =
//    ifilter_mapper_p->RegisterFilter (CLSID_CamStream_Target_Source_Filter, // Filter CLSID. 
//                                      g_wszName,                            // Filter name.
//                                      NULL,                                 // Device moniker. 
//                                      &CLSID_VideoInputDeviceCategory,      // Video capture category.
//                                      g_wszName,                            // Instance data.
//                                      &sudFilterReg);                       // Pointer to filter information.
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IFilterMapper2::RegisterFilter(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto clean;
//  } // end IF
//
//clean:
//  if (ifilter_mapper_p)
//    ifilter_mapper_p->Release ();

  return result;
} // DllRegisterServer

STDAPI
DllUnregisterServer ()
{
  STREAM_TRACE (ACE_TEXT ("::DllUnregisterServer"));

  HRESULT result = E_FAIL;

  result = AMovieDllRegisterServer2 (FALSE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AMovieDllRegisterServer2(false): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

//  IFilterMapper2* ifilter_mapper_p = NULL;
//  result = CoCreateInstance (CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
//                             IID_IFilterMapper2, (void**)&ifilter_mapper_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterMapper2): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return result;
//  } // end IF
//  ACE_ASSERT (ifilter_mapper_p);
//
// result =
//  ifilter_mapper_p->UnregisterFilter (&CLSID_VideoInputDeviceCategory,
//                                      g_wszName,
//                                      CLSID_CamStream_Target_Source_Filter);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IFilterMapper2::RegisterFilter(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto clean;
//  } // end IF
//
//clean:
//  if (ifilter_mapper_p)
//    ifilter_mapper_p->Release ();

  return result;
} // DllUnregisterServer

extern "C" BOOL WINAPI DllEntryPoint (HINSTANCE, ULONG, LPVOID);

BOOL
WINAPI DllMain (HANDLE hModule,
                DWORD  dwReason,
                LPVOID lpReserved)
{
  STREAM_TRACE (ACE_TEXT ("::DllMain"));

  return DllEntryPoint ((HINSTANCE)(hModule), dwReason, lpReserved);
}
