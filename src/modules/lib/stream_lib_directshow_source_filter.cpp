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
#include "stream_lib_directshow_source_filter.h"

////#include "combase.h"
////#include "dllsetup.h"
////#include "guiddef.h"
//#include "streams.h"
////#include "strmif.h"
////#include "uuids.h"
//
//#include "common_time_common.h"
//
////#include "stream_macros.h"
//
//#include "stream_misc_defines.h"
//
//// {F9F62434-535B-4934-A695-BE8D10A4C699}
//DEFINE_GUID (CLSID_ACEStream_Source_Filter,
//0xf9f62434, 0x535b, 0x4934, 0xa6, 0x95, 0xbe, 0x8d, 0x10, 0xa4, 0xc6, 0x99);
//
//// Setup data
//const AMOVIESETUP_MEDIATYPE pin_types =
//{
//  &MEDIATYPE_Video,  // Major type
//  &MEDIASUBTYPE_NULL // Minor type
//};
////const REGFILTERPINS2 sudOpPin = 
////{
////  REG_PINFLAG_B_OUTPUT,  // dwFlags  (also _B_ZERO, _B_MANY, _B_RENDERER)
////  1,                     // cInstances
////  1,                     // nMediaTypes
////  &sudOpPinTypes,        // lpMediaType
////  0,                     // nMediums
////  NULL,                  // lpMedium
////  &PIN_CATEGORY_CAPTURE, // clsPinCategory
////};
////const AdvFilterInfo sudACEStreamax =
////{
////  &CLSID_ACEStream_Source_Filter,          // Filter CLSID
////  MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE, // String name
////  0,                                       // must be 0 for new structure
////  1,                                       // Number pins
////  &sudOpPin,                               // Pin details
////  MERIT_DO_NOT_USE,                        // filter merit
////  &CLSID_VideoInputDeviceCategory,
////};
//const AMOVIESETUP_PIN pin =
//{
//  MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME, // Pin string name
//  FALSE,                                       // Is it rendered
//  TRUE,                                        // Is it an output
//  FALSE,                                       // Can we have none
//  FALSE,                                       // Can we have many
//  &CLSID_NULL,                                 // Connects to filter
//  NULL,                                        // Connects to pin
//  1,                                           // Number of types
//  &pin_types                                   // Pin details
//};
//const AMOVIESETUP_FILTER filter =
//{
//  &CLSID_ACEStream_Source_Filter,            // Filter CLSID
//  MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L, // String name
//  MERIT_DO_NOT_USE,                          // Filter merit
//  1,                                         // Number pins
//  &pin                                       // Pin details
//};
//
//// -----------------------------------------------------------------------------
//
//typedef Stream_Misc_DirectShow_Source_Filter_T<Common_TimePolicy_t,
//                                               ACE_Message_Block,
//                                               ACE_Message_Block> Stream_Misc_DirectShow_Source_Filter_t;
//
//// COM global table of objects in this dll
//CFactoryTemplate g_Templates[] = {
//  { MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L
//  , &CLSID_ACEStream_Source_Filter
//  , Stream_Misc_DirectShow_Source_Filter_t::CreateInstance
//  , NULL
//  , (AMOVIESETUP_FILTER*)&filter }
//};
//int g_cTemplates = sizeof (g_Templates) / sizeof (g_Templates[0]);
//
//// -----------------------------------------------------------------------------
//
//STDAPI DllRegisterServer ()
//{
//  return AMovieDllRegisterServer2 (TRUE);
//} // DllRegisterServer
//
//STDAPI DllUnregisterServer ()
//{
//  return AMovieDllRegisterServer2 (FALSE);
//} // DllUnregisterServer
//
//extern "C" BOOL WINAPI DllEntryPoint (HINSTANCE, ULONG, LPVOID);
//
//BOOL APIENTRY DllMain (HANDLE hModule,
//                       DWORD  dwReason,
//                       LPVOID lpReserved)
//{
//  return DllEntryPoint ((HINSTANCE)(hModule), dwReason, lpReserved);
//}
