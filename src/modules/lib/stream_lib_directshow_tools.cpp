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

#include "amvideo.h"
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
//#include <dsound.h>
//#include <dxva.h>
#include "control.h"
#undef GetObject
#include "evr.h"
#include "fourcc.h"
#include "ksmedia.h"
#include "ksproxy.h"
#include "dmoreg.h"
#include "Dmodshow.h"
#include "dvdmedia.h"
#include "mfapi.h"
#include "mtype.h"
#include "oleauto.h"
#include "qedit.h"
#include "winnt.h"
//#include "reftime.h"
#include "strsafe.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#include "vfwmsgs.h"
#include "wmcodecdsp.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common.h"
#include "common_time_common.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_lib_directsound_tools.h"
#include "stream_lib_tools.h"

// initialize statics
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_DirectShow_Tools::Stream_MediaMajorTypeToStringMap;
ACE_HANDLE Stream_MediaFramework_DirectShow_Tools::logFileHandle = ACE_INVALID_HANDLE;

bool
Stream_MediaFramework_DirectShow_Tools::initialize ()
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

  return true;
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

  HRESULT result = E_FAIL;

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

  result =
    builder_in->SetLogFile (((Stream_MediaFramework_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE) && !fileName_in.empty () ? reinterpret_cast<DWORD_PTR> (Stream_MediaFramework_DirectShow_Tools::logFileHandle)
                                                                                                                                    : NULL));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::SetLogFile(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT (fileName_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    if ((Stream_MediaFramework_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE) &&
        !CloseHandle (Stream_MediaFramework_DirectShow_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
    return;
  } // end IF

  if (fileName_in.empty () &&
      (Stream_MediaFramework_DirectShow_Tools::logFileHandle != ACE_INVALID_HANDLE))
    if (!CloseHandle (Stream_MediaFramework_DirectShow_Tools::logFileHandle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
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

  std::string graph_layout_string;

  Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator_2;
  for (Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator = graphConfiguration_in.begin ();
       iterator != graphConfiguration_in.end ();
       ++iterator)
  {
    graph_layout_string += ACE_TEXT_ALWAYS_CHAR ("\"");
    graph_layout_string +=
      ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ());
    graph_layout_string += ACE_TEXT_ALWAYS_CHAR ("\"");
    iterator_2 = iterator;
    std::advance (iterator_2, 1);
    if (iterator_2 != graphConfiguration_in.end ())
    {
      graph_layout_string += ACE_TEXT_ALWAYS_CHAR (" -- ");
      graph_layout_string +=
        ((*iterator_2).mediaType ? Stream_MediaFramework_DirectShow_Tools::toString (*(*iterator_2).mediaType, true)
                                 : std::string (ACE_TEXT_ALWAYS_CHAR ("NULL")));
      graph_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> ");
    } // end IF
  } // end FOR

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (graph_layout_string.c_str ())));
}

void
Stream_MediaFramework_DirectShow_Tools::clear (Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::clear"));

  for (Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t iterator = graphConfiguration_inout.begin ();
       iterator != graphConfiguration_inout.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_MediaFramework_DirectShow_Tools::delete_ ((*iterator).mediaType, false);
  graphConfiguration_inout.clear ();
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
      reinterpret_cast<struct tWAVEFORMATEX*> (mediaType_in.pbFormat);
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

  struct _PinInfo pin_info_s;
  ACE_OS::memset (&pin_info_s, 0, sizeof (struct _PinInfo));
  HRESULT result_2 = pin_in->QueryPinInfo (&pin_info_s);
  ACE_ASSERT (SUCCEEDED (result_2));
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (pin_info_s.achName));
  pin_info_s.pFilter->Release ();

  return result;
}

IPin*
Stream_MediaFramework_DirectShow_Tools::pin (IBaseFilter* filter_in,
                                             enum _PinDirection direction_in,
                                             unsigned int index_in)
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

  enum _PinDirection pin_direction;
  unsigned int index_i = 0;
  while (S_OK == enumerator_p->Next (1, &result, NULL))
  { ACE_ASSERT (result);
    result_2 = result->QueryDirection (&pin_direction);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      result->Release ();
      enumerator_p->Release ();
      return NULL;
    } // end IF
    if (pin_direction != direction_in)
    {
      result->Release (); result = NULL;
      continue;
    } // end IF
    if (index_i != index_in)
    {
      result->Release (); result = NULL;
      ++index_i;
      continue;
    } // end IF
    break;
  } // end WHILE
  enumerator_p->Release ();

  if (!result)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("0x%@ [\"%s\"]: no %s pin found, aborting\n"),
    //            filter_in,
    //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ()),
    //            ((direction_in == PINDIR_INPUT) ? ACE_TEXT ("input") : ACE_TEXT ("output"))));
  } // end IF

  return result;
}

unsigned int
Stream_MediaFramework_DirectShow_Tools::pins (IBaseFilter* filter_in,
                                              enum _PinDirection direction_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::pin"));

  unsigned int result = 0;

  // sanity check(s)
  ACE_ASSERT (filter_in);

  IEnumPins* enumerator_p = NULL;
  HRESULT result_2 = filter_in->EnumPins (&enumerator_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    return 0;
  } // end IF
  ACE_ASSERT (enumerator_p);

  IPin* pin_p = NULL;
  enum _PinDirection pin_direction;
  while (S_OK == enumerator_p->Next (1, &pin_p, NULL))
  { ACE_ASSERT (pin_p);
    result_2 = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      pin_p->Release ();
      enumerator_p->Release ();
      return 0;
    } // end IF
    if (pin_direction != direction_in)
    {
      pin_p->Release (); pin_p = NULL;
      continue;
    } // end IF
    pin_p->Release (); pin_p = NULL;
    ++result;
  } // end WHILE
  enumerator_p->Release ();

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
                                                               true, // top-to-bottom
                                                               result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::getFirstFormat(\"%s\":\"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ())));
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

  struct _PinInfo pin_info_s;
  ACE_OS::memset (&pin_info_s, 0, sizeof (struct _PinInfo));
  HRESULT result = pin_in->QueryPinInfo (&pin_info_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryPinInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (pin_info_s.pFilter);

  return pin_info_s.pFilter;
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

  IBaseFilter* filter_p = NULL, *filter_2 = NULL;
  HRESULT result = E_FAIL;
  unsigned int number_of_output_pins_i = 0, number_of_input_pins_i = 0;
  IPin* pin_p = NULL, *pin_2 = NULL;
  Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator_2;
  IAMStreamConfig* stream_config_p = NULL;
  struct _AMMediaType* media_type_p = NULL; // previous-
  for (Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t iterator = graphConfiguration_in.begin ();
       iterator != graphConfiguration_in.end ();
       ++iterator)
  {
    iterator_2 = iterator; std::advance (iterator_2, 1);
    if (unlikely (iterator_2 == graphConfiguration_in.end ()))
      break; // done

    media_type_p = ((*iterator).mediaType ? (*iterator).mediaType : media_type_p);

    result =
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
    number_of_output_pins_i =
      Stream_MediaFramework_DirectShow_Tools::pins (filter_p,
                                                    PINDIR_OUTPUT);
    ACE_ASSERT (number_of_output_pins_i >= 1);

    result = builder_in->FindFilterByName ((*iterator_2).filterName.c_str (),
                                           &filter_2);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      filter_p->Release (); filter_p = NULL;
      return false;
    } // end IF
    ACE_ASSERT (filter_2);
    number_of_input_pins_i =
      Stream_MediaFramework_DirectShow_Tools::pins (filter_2,
                                                    PINDIR_INPUT);
    ACE_ASSERT (number_of_input_pins_i >= 1);

    pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                         PINDIR_OUTPUT,
                                                         0);
    ACE_ASSERT (pin_p);
    result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
    if (FAILED (result))
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to IPin::QueryInterface(IAMStreamConfig): \"%s\", continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    else
    { ACE_ASSERT (stream_config_p);
      result = stream_config_p->SetFormat (media_type_p); // *NOTE*: 'NULL' should reset the filter
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to IAMStreamConfig::SetFormat(): \"%s\" (media type was: %s), continuing\n"),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()),
                    (media_type_p ? ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*media_type_p, true).c_str ()) : ACE_TEXT ("NULL"))));
      stream_config_p->Release (); stream_config_p = NULL;
    } // end ELSE

    pin_2 = Stream_MediaFramework_DirectShow_Tools::pin (filter_2,
                                                         PINDIR_INPUT,
                                                         0);
    ACE_ASSERT (pin_2);
    result =
      ((*iterator).connectDirect ? builder_in->ConnectDirect (pin_p,
                                                              pin_2,
                                                              media_type_p)
                                 : pin_p->Connect (pin_2,
                                                   media_type_p));
    if (FAILED (result)) // 0x80040200: VFW_E_INVALIDMEDIATYPE
                         // 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
                         // 0x80040217: VFW_E_CANNOT_CONNECT
                         // 0x8004022A: VFW_E_TYPE_NOT_ACCEPTED
                         // 0x80040255: VFW_E_NO_DECOMPRESSOR
                         // 0x80070057: 
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%spin connection %s/%s[%u] <--> [%u]%s/%s failed (media type was: %s): \"%s\" (0x%x), retrying...\n"),
                  ((*iterator).connectDirect ? ACE_TEXT ("'direct' ") : ACE_TEXT ("")),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()), 0,
                  0, ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_2).c_str ()),
                  ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                  (media_type_p ? ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*media_type_p, true).c_str ()) : ACE_TEXT ("NULL")),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()), result));

      result = builder_in->Connect (pin_p, pin_2);
      if (FAILED (result)) // 0x80040207: VFW_E_NO_ACCEPTABLE_TYPES
                           // 0x80040217: VFW_E_CANNOT_CONNECT
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("'intelligent' pin connection %s/%s[%u] <--> [%u]%s/%s also failed: \"%s\" (0x%x), aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()), 0,
                    0, ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_2).c_str ()),
                    ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()), result));
        pin_p->Release (); pin_p = NULL;
        pin_2->Release (); pin_2 = NULL;

        for (unsigned int i = 0;
             i < number_of_output_pins_i;
             ++i)
        { ACE_ASSERT (!pin_p);
          pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                               PINDIR_OUTPUT,
                                                               i);
          ACE_ASSERT (pin_p);
          Stream_MediaFramework_DirectShow_Tools::dump (pin_p);
          pin_p->Release (); pin_p = NULL;
        } // end FOR
        filter_p->Release (); filter_p = NULL;

        for (unsigned int i = 0;
             i < number_of_input_pins_i;
             ++i)
        {
          ACE_ASSERT (!pin_2);
          pin_2 = Stream_MediaFramework_DirectShow_Tools::pin (filter_2,
                                                               PINDIR_INPUT,
                                                               i);
          ACE_ASSERT (pin_2);
          Stream_MediaFramework_DirectShow_Tools::dump (pin_2);
          pin_2->Release (); pin_2 = NULL;
        } // end FOR
        filter_2->Release (); filter_2 = NULL;

        return false;
      } // end IF
    } // end IF
    struct _AMMediaType media_type_s =
      Stream_MediaFramework_DirectShow_Tools::toFormat (pin_p);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connected \"%s\"[%u] to [%u]\"%s\": %s\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator).filterName.c_str ()), 0, 0,
                ACE_TEXT_WCHAR_TO_TCHAR ((*iterator_2).filterName.c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (media_type_s, true).c_str ())));
    Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
    ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
    filter_p->Release (); filter_p = NULL;
    filter_2->Release (); filter_2 = NULL;
    pin_p->Release (); pin_p = NULL;
    pin_2->Release (); pin_2 = NULL;
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
    { ACE_ASSERT (pin_2);
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
        pin_2->Release (); pin_2 = NULL;
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
  struct _FilterInfo filter_info;
  while (enumerator_p->Next (1, &filter_p, NULL) == S_OK)
  { ACE_ASSERT (filter_p);
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
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName)));

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
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("disconnected \"%s\"...\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));
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

void
Stream_MediaFramework_DirectShow_Tools::shutdown (IGraphBuilder* builder_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::shutdown"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  HRESULT result = E_FAIL;
  IMediaControl* media_control_p = NULL;
  OAFilterState filter_state_e = 0;

  //result = builder_in->Abort ();
  //if (FAILED (result))
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::Abort(): \"%s\", continuing\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

  result = builder_in->QueryInterface (IID_PPV_ARGS (&media_control_p));
  ACE_ASSERT (SUCCEEDED (result) && media_control_p);
  result = media_control_p->GetState (INFINITE,
                                      &filter_state_e);
  ACE_ASSERT (SUCCEEDED (result));
  if ((filter_state_e == State_Paused) ||
      (filter_state_e == State_Running))
  {
    result = media_control_p->Stop ();
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  media_control_p->Release (); media_control_p = NULL;

  if (!Stream_MediaFramework_DirectShow_Tools::disconnect (builder_in))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), continuing\n")));

  if (!Stream_MediaFramework_DirectShow_Tools::clear (builder_in))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::clear(), continuing\n")));
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
    { ACE_ASSERT (filter_p);

      pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                           PINDIR_INPUT);
      if (pin_p)
      {
        pin_p->Release (); pin_p = NULL;
        filter_p->Release (); filter_p = NULL;
        continue;
      } // end IF

      ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
      result = filter_p->QueryFilterInfo (&filter_info);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
        filter_p->Release ();
        enumerator_p->Release ();
        return false;
      } // end IF
      filter_name = filter_info.achName;

      // clean up
      filter_p->Release (); filter_p = NULL;
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
  filter_p->Release (); filter_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ())));

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

    result += ACE_TEXT_ALWAYS_CHAR (" || ");
    result += Stream_MediaFramework_DirectSound_Tools::toString (*waveformatex_p, true);
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
  if (FAILED (result)) // 0x80040209: VFW_E_NOT_CONNECTED
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
Stream_MediaFramework_DirectShow_Tools::isMediaTypeBottomUp (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::isMediaTypeBottomUp"));

  // initialize return value(s)
  bool result = false;

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    video_info_header_p = (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result = video_info_header_p->bmiHeader.biHeight > 0;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result = video_info_header2_p->bmiHeader.biHeight > 0;
  } // end ELSE IF
  else // *TODO*: prevent false negatives !
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
  return result && Stream_MediaFramework_Tools::isRGB (mediaType_in.subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
}

bool
Stream_MediaFramework_DirectShow_Tools::getFirstFormat (IPin* pin_in,
                                                        REFGUID mediaSubType_in,
                                                        bool top_to_bottom_RGB_in,
                                                        struct _AMMediaType*& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::getFirstFormat"));

  // sanity check(s)
  ACE_ASSERT (pin_in);
  ACE_ASSERT (!mediaType_inout);

  bool is_RGB_format_b =
    Stream_MediaFramework_Tools::isRGB (mediaSubType_in,
                                        STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

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
    ACE_ASSERT (media_types_a[0]);

    if (InlineIsEqualGUID (mediaSubType_in, GUID_NULL))
      break;
    if (InlineIsEqualGUID (mediaSubType_in, media_types_a[0]->subtype))
    {
      if (!is_RGB_format_b)
        break;

      bool is_bottom_to_top_b =
        Stream_MediaFramework_DirectShow_Tools::isMediaTypeBottomUp (*media_types_a[0]);
      // *NOTE*: iff the requested subtype is RGB, retrieve top-to-bottom ? : bottom-to-top
      //if ((is_bottom_to_top_b && !top_to_bottom_RGB_in) ||
      //    (!is_bottom_to_top_b && top_to_bottom_RGB_in))
      //  break;
       if ((is_bottom_to_top_b  && top_to_bottom_RGB_in) ||
           (!is_bottom_to_top_b && !top_to_bottom_RGB_in))
      {
        struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
        struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
        if (InlineIsEqualGUID (media_types_a[0]->formattype, FORMAT_VideoInfo))
        {
          video_info_header_p =
            (struct tagVIDEOINFOHEADER*)media_types_a[0]->pbFormat;
          video_info_header_p->bmiHeader.biHeight =
            -video_info_header_p->bmiHeader.biHeight;
        } // end IF
        else if (InlineIsEqualGUID (media_types_a[0]->formattype, FORMAT_VideoInfo2))
        {
          video_info_header2_p =
            (struct tagVIDEOINFOHEADER2*)media_types_a[0]->pbFormat;
          video_info_header2_p->bmiHeader.biHeight =
            -video_info_header2_p->bmiHeader.biHeight;
        }    // end ELSE IF
        else // *TODO*: prevent false negatives !
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), continuing\n"),
                      ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (media_types_a[0]->formattype).c_str ())));
      } // end IF
      break;
    } // end IF
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_types_a[0], true);
  } while (true);
  enumerator_p->Release (); enumerator_p = NULL;

  if (media_types_a[0])
  {
    mediaType_inout =
      Stream_MediaFramework_DirectShow_Tools::copy (*media_types_a[0]);
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_types_a[0], true);
  } // end IF

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
#endif // _DEBUG

continue_:
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_types_a[0]);
  } while (true);
  enumerator_p->Release (); enumerator_p = NULL;

  return result;
}

bool
Stream_MediaFramework_DirectShow_Tools::copy (const struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_in,
                                              struct Stream_MediaFramework_DirectShow_AudioVideoFormat& result_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::copy"));

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (result_out.audio);
  Stream_MediaFramework_DirectShow_Tools::free (result_out.video);

  Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in.audio,
                                                result_out.audio);
  Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in.video,
                                                result_out.video);

  return true;
}

void
Stream_MediaFramework_DirectShow_Tools::free (struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::free"));

  Stream_MediaFramework_DirectShow_Tools::free (mediaType_inout.audio);
  Stream_MediaFramework_DirectShow_Tools::free (mediaType_inout.video);
}

void
Stream_MediaFramework_DirectShow_Tools::free (Stream_MediaFramework_DirectShow_AudioVideoFormats_t& mediaTypes_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::free"));

  for (Stream_MediaFramework_DirectShow_AudioVideoFormatsIterator_t iterator = mediaTypes_inout.begin ();
       iterator != mediaTypes_inout.end ();
       ++iterator)
    Stream_MediaFramework_DirectShow_Tools::free (*iterator);
}

std::string
Stream_MediaFramework_DirectShow_Tools::toString (const struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_in,
                                                  bool condensed_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toString"));

  std::string result = ACE_TEXT_ALWAYS_CHAR ("AUDIO:\n");
  result += Stream_MediaFramework_DirectShow_Tools::toString (mediaType_in.audio,
                                                              condensed_in);
  result += ACE_TEXT_ALWAYS_CHAR ("\nVIDEO:\n");
  result += Stream_MediaFramework_DirectShow_Tools::toString (mediaType_in.video,
                                                              condensed_in);
  return result;
}

struct _AMMediaType*
Stream_MediaFramework_DirectShow_Tools::copy (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::copy"));

  // initialize return value(s)
  struct _AMMediaType* result_p = NULL;
  ACE_NEW_NORETURN (result_p,
                    struct _AMMediaType ());
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
    delete result_p; result_p = NULL;
    return NULL;
  } // end IF

  return result_p;
}

bool
Stream_MediaFramework_DirectShow_Tools::copy (const struct _AMMediaType& mediaType_in,
                                              struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::copy"));

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (mediaType_out);
  ACE_OS::memset (&mediaType_out, 0, sizeof (struct _AMMediaType));

  HRESULT result = CopyMediaType (&mediaType_out,
                                  &mediaType_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CopyMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF

  return true;
}

void
Stream_MediaFramework_DirectShow_Tools::delete_ (struct _AMMediaType*& mediaType_inout,
                                                 bool useDeleteMediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::delete_"));

  // sanity check(s)
  ACE_ASSERT (mediaType_inout);

  if (useDeleteMediaType_in)
  {
    DeleteMediaType (mediaType_inout); mediaType_inout = NULL;
  } // end IF
  else
  {
    FreeMediaType (*mediaType_inout);
    delete mediaType_inout; mediaType_inout = NULL;
  } // end ELSE
}

void
Stream_MediaFramework_DirectShow_Tools::free (struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::free"));

  FreeMediaType (mediaType_in);
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
  else
  {
    if (mediaType_in.lSampleSize != mediaType2_in.lSampleSize)
      return false;
  } // end ELSE
continue_:

  return true;
}

bool
Stream_MediaFramework_DirectShow_Tools::isVideoFormat (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::isVideoFormat"));

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo) &&
      (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER)))
    return true;
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2) &&
           (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER2)))
    return true;
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx) &&
           (mediaType_in.cbFormat == sizeof (struct tWAVEFORMATEX)))
    return false;
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media type format (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (mediaType_in).c_str ())));

  return false; // <-- false negative
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
  unsigned int frames_per_second_i = 0;
  if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_inout.pbFormat;
    video_info_header_p->bmiHeader.biBitCount =
      Stream_MediaFramework_Tools::toBitCount (mediaType_inout.subtype,
                                               STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
    video_info_header_p->bmiHeader.biCompression =
      (Stream_MediaFramework_Tools::isCompressedVideo (mediaType_inout.subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ? fourcc_map.GetFOURCC ()
                                                                                         : BI_RGB);
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    frames_per_second_i =
      /*UNITS*/10000000 / static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * frames_per_second_i) * 8;
    mediaType_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_inout.pbFormat;
    video_info_header2_p->bmiHeader.biBitCount =
      Stream_MediaFramework_Tools::toBitCount (mediaType_inout.subtype,
                                               STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
    video_info_header2_p->bmiHeader.biCompression =
      (Stream_MediaFramework_Tools::isCompressedVideo (mediaType_inout.subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ? fourcc_map.GetFOURCC ()
                                                                                         : BI_RGB);
    video_info_header2_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header2_p->bmiHeader);
    frames_per_second_i =
      /*UNITS*/10000000 / static_cast<unsigned int> (video_info_header2_p->AvgTimePerFrame);
    video_info_header2_p->dwBitRate =
      (video_info_header2_p->bmiHeader.biSizeImage * frames_per_second_i) * 8;
    mediaType_inout.lSampleSize = video_info_header2_p->bmiHeader.biSizeImage;
  } // end ELSE IF
  else if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_WaveFormatEx))
  {
    struct tWAVEFORMATEX* audio_info_header_p =
      (struct tWAVEFORMATEX*)mediaType_inout.pbFormat;
    if (audio_info_header_p->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
      WAVEFORMATEXTENSIBLE* waveformatextensible_p =
        (WAVEFORMATEXTENSIBLE*)mediaType_inout.pbFormat;
      waveformatextensible_p->SubFormat = mediaSubType_in;
      if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IEEE_FLOAT))
      {
        waveformatextensible_p->Format.wBitsPerSample = 32;
        waveformatextensible_p->Format.nBlockAlign =
          (waveformatextensible_p->Format.wBitsPerSample / 8) * waveformatextensible_p->Format.nChannels;
        waveformatextensible_p->Format.nAvgBytesPerSec =
          waveformatextensible_p->Format.nBlockAlign * waveformatextensible_p->Format.nSamplesPerSec;
        mediaType_inout.lSampleSize =
          waveformatextensible_p->Format.nBlockAlign;
      } // end IF
    } // end IF
    else
    {
      if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_PCM))
        audio_info_header_p->wFormatTag = WAVE_FORMAT_PCM;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IEEE_FLOAT))
      {
        audio_info_header_p->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        audio_info_header_p->wBitsPerSample = 32;
        audio_info_header_p->nBlockAlign =
          (audio_info_header_p->wBitsPerSample / 8) * audio_info_header_p->nChannels;
        audio_info_header_p->nAvgBytesPerSec =
          audio_info_header_p->nBlockAlign * audio_info_header_p->nSamplesPerSec;
        mediaType_inout.lSampleSize = audio_info_header_p->nBlockAlign;
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown audio media format (was: \"%s\"), returning\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (mediaSubType_in).c_str ())));
      } // end ELSE
    } // end ELSE
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media formattype (was: \"%s\"), returning\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_inout.formattype).c_str ())));
  } // end ELSE
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
      /*UNITS*/10000000 / static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
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
      /*UNITS*/10000000 / static_cast<unsigned int> (video_info_header2_p->AvgTimePerFrame);
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
  //ACE_ASSERT (frame_size_i); // *NOTE*: biBitCount may not be set (i.e. 0) for compressed formats
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
    video_info_header_p->AvgTimePerFrame = /*UNITS*/ 10000000 / frameRate_in;
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * frameRate_in) * 8;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_inout.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_inout.pbFormat;
    video_info_header2_p->AvgTimePerFrame = /*UNITS*/ 10000000 / frameRate_in;
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

  if (unlikely (!Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in,
                                                               result_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    ACE_OS::memset (&result_s, 0, sizeof (struct _AMMediaType));
    return result_s;
  } // end IF

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
    BOOL result_3 = SetRectEmpty (&video_info_header_p->rcSource);
    ACE_ASSERT (result_3);
    result_3 = SetRectEmpty (&video_info_header_p->rcTarget);
    // *NOTE*: empty --> fill entire buffer
    ACE_ASSERT (result_3);
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
    BOOL result_3 = SetRectEmpty (&video_info_header_p->rcSource);
    ACE_ASSERT (result_3);
    result_3 = SetRectEmpty (&video_info_header_p->rcTarget);
    // *NOTE*: empty --> fill entire buffer
    ACE_ASSERT (result_3);
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

struct tWAVEFORMATEX*
Stream_MediaFramework_DirectShow_Tools::toWaveFormatEx (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toWaveFormatEx"));

  struct tWAVEFORMATEX* result_p = NULL;

  // sanity check(s)
  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx));
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (mediaType_in.pbFormat);
  ACE_ASSERT (waveformatex_p);

  result_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (CoTaskMemAlloc (sizeof (struct tWAVEFORMATEX) + waveformatex_p->cbSize));
  if (unlikely (!result_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    return NULL;
  } // end IF
  ACE_OS::memcpy (result_p,
                  waveformatex_p,
                  sizeof (struct tWAVEFORMATEX) + waveformatex_p->cbSize);

  return result_p;
}

struct _GUID
Stream_MediaFramework_DirectShow_Tools::compressionToSubType (DWORD biCompression_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::compressionToSubType"));

  struct _GUID result = GUID_NULL;

  FOURCCMap fourcc_map (biCompression_in);
  result = fourcc_map;

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("converted %u compression to \"%s\" subtype\n"),
  //            biCompression_in,
  //            Stream_MediaFramework_Tools::mediaSubTypeToString (result, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()));

  return result;
}

void
Stream_MediaFramework_DirectShow_Tools::toBitmapInfo (const struct _AMMediaType& mediaType_in,
                                                      struct tagBITMAPINFO& bitmapInfo_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toBitmapInfo"));

  ACE_OS::memset (&bitmapInfo_out, 0, sizeof (struct tagBITMAPINFO));

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    struct tagVIDEOINFOHEADER* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
    bitmapInfo_out.bmiHeader = video_info_header_p->bmiHeader;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  { ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
    struct tagVIDEOINFOHEADER2* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_in.pbFormat);
    bitmapInfo_out.bmiHeader = video_info_header_p->bmiHeader;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
  } // end ELSE
}

struct _GUID
Stream_MediaFramework_DirectShow_Tools::toSubType (const struct tWAVEFORMATEX& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toSubType"));

  struct _GUID result = GUID_NULL;

  switch (format_in.wFormatTag)
  {
    case WAVE_FORMAT_PCM:
    {
      result = MEDIASUBTYPE_PCM;
      break;
    }
    case WAVE_FORMAT_IEEE_FLOAT:
    {
      result = MEDIASUBTYPE_IEEE_FLOAT;
      break;
    }
    case WAVE_FORMAT_EXTENSIBLE:
    {
      const WAVEFORMATEXTENSIBLE* waveformatextensible_p =
        reinterpret_cast<const WAVEFORMATEXTENSIBLE*> (&format_in);
      result = waveformatextensible_p->SubFormat;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown format type (was: %d), aborting\n"),
                  format_in.wFormatTag));
      return GUID_NULL;
    }
  } // end SWITCH

  return result;
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
  } // end ELSE IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
    result += ACE_TEXT_ALWAYS_CHAR ("---\n");
    result +=
      Stream_MediaFramework_DirectSound_Tools::toString (*waveformatex_p, false);
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
Stream_MediaFramework_DirectShow_Tools::toRowStride (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toRowStride"));

  unsigned int result = 0;

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result =
      (video_info_header_p->bmiHeader.biWidth * (video_info_header_p->bmiHeader.biBitCount / 8));
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result =
      (video_info_header2_p->bmiHeader.biWidth * (video_info_header2_p->bmiHeader.biBitCount / 8));
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

  // sanity check(s)
  ACE_ASSERT (mediaType_in.pbFormat);

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
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
    result = waveformatex_p->nSamplesPerSec;
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

unsigned int
Stream_MediaFramework_DirectShow_Tools::toFrameBits (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toFrameBits"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in.pbFormat);

  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p =
      (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    return video_info_header_p->bmiHeader.biBitCount;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    struct tagVIDEOINFOHEADER2* video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    return video_info_header2_p->bmiHeader.biBitCount;
  } // end ELSE IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  {
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
    return waveformatex_p->wBitsPerSample;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));
  } // end ELSE

  return 0;
}

unsigned int
Stream_MediaFramework_DirectShow_Tools::toChannels (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toBitrate"));

  // sanity check(s)
  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (mediaType_in.pbFormat);

  struct tWAVEFORMATEX* waveformatex_p =
    (struct tWAVEFORMATEX*)mediaType_in.pbFormat;

  return waveformatex_p->nChannels;
}

void
Stream_MediaFramework_DirectShow_Tools::getAudioRendererStatistics (IFilterGraph* graph_in,
                                                                    Stream_MediaFrameWork_DirectSound_Statistics_t& statistic_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::getAudioRendererStatistics"));

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
Stream_MediaFramework_DirectShow_Tools::toString (enum _AM_AUDIO_RENDERER_STAT_PARAM parameter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toString"));

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

#if defined (FFMPEG_SUPPORT)
struct _AMMediaType*
Stream_MediaFramework_DirectShow_Tools::to (const struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::to"));

  // initialize return value(s)
  struct _AMMediaType* result_p = NULL;
  ACE_NEW_NORETURN (result_p,
                    struct _AMMediaType ());
  if (unlikely (!result_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  BOOL result_2 = FALSE;
  result_p->majortype = MEDIATYPE_Audio;
  result_p->subtype =
    Stream_MediaFramework_Tools::AVSampleFormatToMediaSubType (mediaType_in.format);
  result_p->bFixedSizeSamples = TRUE;
  result_p->bTemporalCompression = FALSE;
  result_p->formattype = FORMAT_WaveFormatEx;
  result_p->cbFormat = sizeof (struct tWAVEFORMATEX);
  result_p->pbFormat =
    reinterpret_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tWAVEFORMATEX)));
  if (unlikely (!result_p->pbFormat))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    delete result_p; result_p = NULL;
    return NULL;
  } // end IF
  ACE_OS::memset (result_p->pbFormat, 0, sizeof (struct tWAVEFORMATEX));
  struct tWAVEFORMATEX* wave_format_ex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (result_p->pbFormat);
  wave_format_ex_p->wFormatTag =
    Stream_MediaFramework_Tools::AVSampleFormatToFormatTag (mediaType_in.format);
  wave_format_ex_p->nChannels = mediaType_in.channels;
  wave_format_ex_p->nSamplesPerSec = mediaType_in.sampleRate;
  wave_format_ex_p->wBitsPerSample =
    av_get_bytes_per_sample (mediaType_in.format) * 8;
  wave_format_ex_p->nBlockAlign =
    (wave_format_ex_p->wBitsPerSample / 8) * wave_format_ex_p->nChannels;
  wave_format_ex_p->nAvgBytesPerSec =
    wave_format_ex_p->nBlockAlign * wave_format_ex_p->nSamplesPerSec;
  result_p->lSampleSize = wave_format_ex_p->nBlockAlign;

  return result_p;
}

struct _AMMediaType*
Stream_MediaFramework_DirectShow_Tools::to (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::to"));

  // initialize return value(s)
  struct _AMMediaType* result_p = NULL;
  ACE_NEW_NORETURN (result_p,
                    struct _AMMediaType ());
  if (unlikely (!result_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return NULL;
  } // end IF

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
  if (unlikely (!result_p->pbFormat))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    delete result_p; result_p = NULL;
    return NULL;
  } // end IF
  ACE_OS::memset (result_p->pbFormat, 0, sizeof (struct tagVIDEOINFOHEADER));
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (result_p->pbFormat);
  // *NOTE*: empty --> use entire video
  result_2 = SetRectEmpty (&video_info_header_p->rcSource);
  ACE_ASSERT (result_2);
  result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
  // *NOTE*: empty --> fill entire buffer
  ACE_ASSERT (result_2);
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

enum AVSampleFormat
Stream_MediaFramework_DirectShow_Tools::toAVSampleFormat (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::toAVSampleFormat"));

  enum AVSampleFormat result = AV_SAMPLE_FMT_NONE;

  // sanity check(s)
  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx));

  struct tWAVEFORMATEX* waveformatex_p =
    (struct tWAVEFORMATEX*)mediaType_in.pbFormat;
  switch (waveformatex_p->wBitsPerSample)
  {
    case 8:
      result = AV_SAMPLE_FMT_U8; break;
    case 16:
      result = AV_SAMPLE_FMT_S16; break;
    case 32:
    {
      if (mediaType_in.subtype == MEDIASUBTYPE_PCM)
        result = AV_SAMPLE_FMT_S32;
      else if (mediaType_in.subtype == MEDIASUBTYPE_IEEE_FLOAT)
        result = AV_SAMPLE_FMT_FLT;
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.subtype).c_str ())));
      break;
    }
    case 64:
    {
      result = AV_SAMPLE_FMT_S64;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown bit/sample (was: %d), aborting\n"),
                  waveformatex_p->wBitsPerSample));
      break;
    }
  } // end SWITCH

  return result;
}

enum AVPixelFormat
Stream_MediaFramework_DirectShow_Tools::mediaSubTypeToAVPixelFormat (REFGUID mediaSubType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::mediaSubTypeToAVPixelFormat"));

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
  else if (mediaSubType_in == MEDIASUBTYPE_RGB4)
    return AV_PIX_FMT_RGB4;
  else if (mediaSubType_in == MEDIASUBTYPE_RGB8)
    return AV_PIX_FMT_RGB8;
  else if (mediaSubType_in == MEDIASUBTYPE_RGB555)
   return AV_PIX_FMT_RGB555;
  else if (mediaSubType_in == MEDIASUBTYPE_RGB565)
    return AV_PIX_FMT_RGB565;
  else if (mediaSubType_in == MEDIASUBTYPE_RGB24)
    return AV_PIX_FMT_RGB24;
  else if (mediaSubType_in == MEDIASUBTYPE_RGB32)
    return AV_PIX_FMT_RGB32;
  // uncompressed RGB (alpha)
  //MEDIASUBTYPE_ARGB1555
  else if (mediaSubType_in == MEDIASUBTYPE_ARGB32)
    return AV_PIX_FMT_ARGB;
  //MEDIASUBTYPE_ARGB4444
  //MEDIASUBTYPE_A2R10G10B10
  //MEDIASUBTYPE_A2B10G10R10

  // video mixing renderer (VMR-7)
  else if (mediaSubType_in == MEDIASUBTYPE_RGB32_D3D_DX7_RT)
    return AV_PIX_FMT_RGB32;
  else if (mediaSubType_in == MEDIASUBTYPE_RGB16_D3D_DX7_RT)
    return AV_PIX_FMT_RGB565;
  else if (mediaSubType_in == MEDIASUBTYPE_ARGB32_D3D_DX7_RT)
    return AV_PIX_FMT_ARGB;
  else if (mediaSubType_in == MEDIASUBTYPE_ARGB4444_D3D_DX7_RT)
    return AV_PIX_FMT_RGB444;
  //MEDIASUBTYPE_ARGB1555_D3D_DX7_RT
  // video mixing renderer (VMR-9)
  else if (mediaSubType_in == MEDIASUBTYPE_RGB32_D3D_DX9_RT)
    return AV_PIX_FMT_RGB32;
  else if (mediaSubType_in == MEDIASUBTYPE_RGB16_D3D_DX9_RT)
    return AV_PIX_FMT_RGB565;
  else if (mediaSubType_in == MEDIASUBTYPE_ARGB32_D3D_DX9_RT)
    return AV_PIX_FMT_ARGB;
  else if (mediaSubType_in == MEDIASUBTYPE_ARGB4444_D3D_DX9_RT)
    return AV_PIX_FMT_RGB444;
  //MEDIASUBTYPE_ARGB1555_D3D_DX9_RT

  // YUV video
  else if (mediaSubType_in == MEDIASUBTYPE_AYUV)
    return AV_PIX_FMT_AYUV64;
  else if (mediaSubType_in == MEDIASUBTYPE_YUY2)
    return AV_PIX_FMT_YUYV422;
  else if (mediaSubType_in == MEDIASUBTYPE_UYVY)
    return AV_PIX_FMT_UYVY422;
  else if (mediaSubType_in == MEDIASUBTYPE_IMC1)
    return AV_PIX_FMT_YUV420P;
  //MEDIASUBTYPE_IMC2
  //MEDIASUBTYPE_IMC3
  //MEDIASUBTYPE_IMC4
  //MEDIASUBTYPE_YV12
  else if (mediaSubType_in == MEDIASUBTYPE_NV12)
    return AV_PIX_FMT_NV12;
  // other YUV
  //MEDIASUBTYPE_I420
  //MEDIASUBTYPE_IF09
  //MEDIASUBTYPE_IYUV
  //MEDIASUBTYPE_Y211
  else if (mediaSubType_in == MEDIASUBTYPE_Y411)
    return AV_PIX_FMT_YUV411P;
  else if (mediaSubType_in == MEDIASUBTYPE_Y41P)
    return AV_PIX_FMT_YUV411P;
  else if (mediaSubType_in == MEDIASUBTYPE_YVU9)
    return AV_PIX_FMT_YUV420P9;
  else if (mediaSubType_in == MEDIASUBTYPE_YVYU)
    return AV_PIX_FMT_YVYU422;
  else if (mediaSubType_in == MEDIASUBTYPE_YUYV)
    return AV_PIX_FMT_YUYV422;

  // miscellaneous
  //MEDIASUBTYPE_CFCC
  //MEDIASUBTYPE_CLJR
  //MEDIASUBTYPE_CPLA
  //MEDIASUBTYPE_CLPL
  //MEDIASUBTYPE_IJPG
  //MEDIASUBTYPE_MDVF
  else if (mediaSubType_in == MEDIASUBTYPE_MJPG)
    return AV_PIX_FMT_YUVJ422P;
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
  else if (mediaSubType_in == MFVideoFormat_RGB32)
    return AV_PIX_FMT_RGB32;
  else if (mediaSubType_in == MFVideoFormat_ARGB32)
    return AV_PIX_FMT_ARGB;
  else if (mediaSubType_in == MFVideoFormat_RGB24)
    return AV_PIX_FMT_RGB24;
  else if (mediaSubType_in == MFVideoFormat_RGB555)
    return AV_PIX_FMT_RGB555;
  else if (mediaSubType_in == MFVideoFormat_RGB565)
    return AV_PIX_FMT_RGB565;
  else if (mediaSubType_in == MFVideoFormat_RGB8)
    return AV_PIX_FMT_RGB8;
  else if (mediaSubType_in == MFVideoFormat_AI44)
    return AV_PIX_FMT_YUV444P;
  else if (mediaSubType_in == MFVideoFormat_AYUV)
    return AV_PIX_FMT_AYUV64;
  else if (mediaSubType_in == MFVideoFormat_YUY2)
    return AV_PIX_FMT_YUYV422;
  else if (mediaSubType_in == MFVideoFormat_YVYU)
    return AV_PIX_FMT_YVYU422;
  else if (mediaSubType_in == MFVideoFormat_YVU9)
    return AV_PIX_FMT_YUV420P9;
  else if (mediaSubType_in == MFVideoFormat_UYVY)
    return AV_PIX_FMT_UYVY422;
  //MFVideoFormat_NV11
  else if (mediaSubType_in == MFVideoFormat_NV12)
    return AV_PIX_FMT_NV12;
  // *TODO*: this is wrong...
  else if (mediaSubType_in == MFVideoFormat_YV12)
    return AV_PIX_FMT_NV21;
  //MFVideoFormat_I420
  // *TODO*: endianness of the bytestream may not be that of the host
  else if (mediaSubType_in == MFVideoFormat_IYUV)
    return AV_PIX_FMT_YUV420P16;
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
#endif // WINVER >= _WIN32_WINNT_WIN8
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
#endif // FFMPEG_SUPPORT

#if defined(SOX_SUPPORT)
void
Stream_MediaFramework_DirectShow_Tools::to (const struct _AMMediaType& mediaType_in,
                                            struct sox_encodinginfo_t& encoding_inout,
                                            struct sox_signalinfo_t& signalInfo_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Tools::to"));

  // initialize return value(s)
  ACE_OS::memset (&encoding_inout, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&signalInfo_inout, 0, sizeof (struct sox_signalinfo_t));

  // sanit check(s)
  if (!InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid format type (was: \"%s\"), returning\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_in.formattype).c_str ())));
    return;
  } // end IF
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (mediaType_in.pbFormat);
  ACE_ASSERT (waveformatex_p);

  encoding_inout.bits_per_sample = waveformatex_p->wBitsPerSample;
  switch (waveformatex_p->wFormatTag)
  {
    case WAVE_FORMAT_PCM:
    {
      encoding_inout.encoding =
        (waveformatex_p->wBitsPerSample == 8 ? SOX_ENCODING_UNSIGNED
                                             : SOX_ENCODING_SIGN2);
      break;
    }
    case WAVE_FORMAT_IEEE_FLOAT:
    {
      encoding_inout.encoding = SOX_ENCODING_FLOAT;
      break;
    }
    case WAVE_FORMAT_EXTENSIBLE:
    {
      WAVEFORMATEXTENSIBLE* waveformatextensible_p =
        reinterpret_cast<WAVEFORMATEXTENSIBLE*> (mediaType_in.pbFormat);
      ACE_ASSERT (waveformatextensible_p);
      if (InlineIsEqualGUID (waveformatextensible_p->SubFormat, MEDIASUBTYPE_PCM))
        encoding_inout.encoding =
          (waveformatex_p->wBitsPerSample == 8 ? SOX_ENCODING_UNSIGNED
                                               : SOX_ENCODING_SIGN2);
      else if (InlineIsEqualGUID (waveformatextensible_p->SubFormat, MEDIASUBTYPE_IEEE_FLOAT))
        encoding_inout.encoding = SOX_ENCODING_FLOAT;
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown subformat (was: \"%s\"), returning\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
        return;
      } // end ELSE
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown format tag (was: %d), returning\n"),
                  waveformatex_p->wFormatTag));
      return;
    } // end IF
  } // end SWITCH

  signalInfo_inout.channels = waveformatex_p->nChannels;
  signalInfo_inout.precision = waveformatex_p->wBitsPerSample;
  signalInfo_inout.rate = waveformatex_p->nSamplesPerSec;
}
#endif // SOX_SUPPORT
