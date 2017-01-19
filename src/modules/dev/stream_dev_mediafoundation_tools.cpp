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

#include "stream_dev_mediafoundation_tools.h"

#include <sstream>

#include <ace/Log_Msg.h>

#include <oleauto.h>

#include <dmoreg.h>
#include <dshow.h>
#include <dsound.h>
#include <dvdmedia.h>
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
#include "stream_dev_directshow_tools.h"
#include "stream_dev_tools.h"

// initialize statics
Stream_Module_Device_MediaFoundation_Tools::GUID2STRING_MAP_T Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap;
Stream_Module_Device_MediaFoundation_Tools::GUID2STRING_MAP_T Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap;

void
Stream_Module_Device_MediaFoundation_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::initialize"));

  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Default, ACE_TEXT_ALWAYS_CHAR ("MF default")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Audio, ACE_TEXT_ALWAYS_CHAR ("MF audio")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Video, ACE_TEXT_ALWAYS_CHAR ("MF video")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Protected, ACE_TEXT_ALWAYS_CHAR ("MF protected")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_SAMI, ACE_TEXT_ALWAYS_CHAR ("MF SAMI")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Script, ACE_TEXT_ALWAYS_CHAR ("MF script")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Image, ACE_TEXT_ALWAYS_CHAR ("MF image")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_HTML, ACE_TEXT_ALWAYS_CHAR ("MF HTML")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Binary, ACE_TEXT_ALWAYS_CHAR ("MF binary")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_FileTransfer, ACE_TEXT_ALWAYS_CHAR ("MF file transfer")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaMajorType2StringMap.insert (std::make_pair (MFMediaType_Stream, ACE_TEXT_ALWAYS_CHAR ("MF stream")));

  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB32, ACE_TEXT_ALWAYS_CHAR ("RGB32")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_ARGB32, ACE_TEXT_ALWAYS_CHAR ("ARGB32")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB24, ACE_TEXT_ALWAYS_CHAR ("RGB24")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB555, ACE_TEXT_ALWAYS_CHAR ("RGB555")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB565, ACE_TEXT_ALWAYS_CHAR ("RGB565")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_RGB8, ACE_TEXT_ALWAYS_CHAR ("RGB8")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_AI44, ACE_TEXT_ALWAYS_CHAR ("AI44")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_AYUV, ACE_TEXT_ALWAYS_CHAR ("AYUV")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YUY2, ACE_TEXT_ALWAYS_CHAR ("YUY2")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YVYU, ACE_TEXT_ALWAYS_CHAR ("YVYU")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YVU9, ACE_TEXT_ALWAYS_CHAR ("YVU9")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_UYVY, ACE_TEXT_ALWAYS_CHAR ("UYVY")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_NV11, ACE_TEXT_ALWAYS_CHAR ("NV11")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_NV12, ACE_TEXT_ALWAYS_CHAR ("NV12")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_YV12, ACE_TEXT_ALWAYS_CHAR ("YV12")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_I420, ACE_TEXT_ALWAYS_CHAR ("I420")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_IYUV, ACE_TEXT_ALWAYS_CHAR ("IYUV")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y210, ACE_TEXT_ALWAYS_CHAR ("Y210")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y216, ACE_TEXT_ALWAYS_CHAR ("Y216")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y410, ACE_TEXT_ALWAYS_CHAR ("Y410")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y416, ACE_TEXT_ALWAYS_CHAR ("Y416")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y41P, ACE_TEXT_ALWAYS_CHAR ("Y41P")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y41T, ACE_TEXT_ALWAYS_CHAR ("Y41T")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_Y42T, ACE_TEXT_ALWAYS_CHAR ("Y42T")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P210, ACE_TEXT_ALWAYS_CHAR ("P210")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P216, ACE_TEXT_ALWAYS_CHAR ("P216")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P010, ACE_TEXT_ALWAYS_CHAR ("P010")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_P016, ACE_TEXT_ALWAYS_CHAR ("P016")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v210, ACE_TEXT_ALWAYS_CHAR ("V210")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v216, ACE_TEXT_ALWAYS_CHAR ("V216")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_v410, ACE_TEXT_ALWAYS_CHAR ("V410")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP43, ACE_TEXT_ALWAYS_CHAR ("MP43")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP4S, ACE_TEXT_ALWAYS_CHAR ("MP4S")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_M4S2, ACE_TEXT_ALWAYS_CHAR ("M4S2")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MP4V, ACE_TEXT_ALWAYS_CHAR ("MP4V")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV1, ACE_TEXT_ALWAYS_CHAR ("WMV1")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV2, ACE_TEXT_ALWAYS_CHAR ("WMV2")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WMV3, ACE_TEXT_ALWAYS_CHAR ("WMV3")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_WVC1, ACE_TEXT_ALWAYS_CHAR ("WVC1")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MSS1, ACE_TEXT_ALWAYS_CHAR ("MSS1")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MSS2, ACE_TEXT_ALWAYS_CHAR ("MSS2")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MPG1, ACE_TEXT_ALWAYS_CHAR ("MPG1")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVSL, ACE_TEXT_ALWAYS_CHAR ("DVSL")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVSD, ACE_TEXT_ALWAYS_CHAR ("DVSD")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVHD, ACE_TEXT_ALWAYS_CHAR ("DVHD")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DV25, ACE_TEXT_ALWAYS_CHAR ("DV25")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DV50, ACE_TEXT_ALWAYS_CHAR ("DV50")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVH1, ACE_TEXT_ALWAYS_CHAR ("DVH1")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_DVC,  ACE_TEXT_ALWAYS_CHAR ("DVC")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H264, ACE_TEXT_ALWAYS_CHAR ("H264")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MJPG, ACE_TEXT_ALWAYS_CHAR ("MJPG")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_420O, ACE_TEXT_ALWAYS_CHAR ("420O")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_HEVC, ACE_TEXT_ALWAYS_CHAR ("HEVC")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_HEVC_ES, ACE_TEXT_ALWAYS_CHAR ("HEVC_ES")));
#if (WINVER >= _WIN32_WINNT_WIN8)
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H263, ACE_TEXT_ALWAYS_CHAR ("H263")));
#endif
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_H264_ES, ACE_TEXT_ALWAYS_CHAR ("H264_ES")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFVideoFormat_MPEG2, ACE_TEXT_ALWAYS_CHAR ("MPEG2")));

  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_PCM, ACE_TEXT_ALWAYS_CHAR ("PCM")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Float, ACE_TEXT_ALWAYS_CHAR ("IEEE_FLOAT")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_DTS, ACE_TEXT_ALWAYS_CHAR ("DTS")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3_SPDIF, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3_SPDIF")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_DRM, ACE_TEXT_ALWAYS_CHAR ("DRM")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudioV8, ACE_TEXT_ALWAYS_CHAR ("WMAudioV8")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudioV9, ACE_TEXT_ALWAYS_CHAR ("WMAudioV9")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMAudio_Lossless, ACE_TEXT_ALWAYS_CHAR ("WMAudio_Lossless")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_WMASPDIF, ACE_TEXT_ALWAYS_CHAR ("WMASPDIF")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MSP1, ACE_TEXT_ALWAYS_CHAR ("MSP1")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MP3, ACE_TEXT_ALWAYS_CHAR ("MP3")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_MPEG, ACE_TEXT_ALWAYS_CHAR ("MPEG")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AAC, ACE_TEXT_ALWAYS_CHAR ("AAC")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_ADTS, ACE_TEXT_ALWAYS_CHAR ("ADTS")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_NB, ACE_TEXT_ALWAYS_CHAR ("AMR_NB")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_WB, ACE_TEXT_ALWAYS_CHAR ("AMR_WB")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_AMR_WP, ACE_TEXT_ALWAYS_CHAR ("AMR_WP")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_AC3, ACE_TEXT_ALWAYS_CHAR ("Dolby_AC3")));
  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.insert (std::make_pair (MFAudioFormat_Dolby_DDPlus, ACE_TEXT_ALWAYS_CHAR ("Dolby_DDPlus")));
}

//void
//Stream_Module_Device_MediaFoundation_Tools::dump (IMFSourceReader* IMFSourceReader_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::dump"));
//
//  // sanity check(s)
//  ACE_ASSERT (IMFSourceReader_in);
//
//  HRESULT result = S_OK;
//  DWORD count = 0;
//  IMFMediaType* media_type_p = NULL;
//  while (true)
//  {
//    media_type_p = NULL;
//    result =
//      IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                              count,
//                                              &media_type_p);
//    if (FAILED (result)) break;
//
//    ACE_DEBUG ((LM_INFO,
//                ACE_TEXT ("#%d: %s\n"),
//                count + 1,
//                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (media_type_p).c_str ())));
//
//    // clean up
//    media_type_p->Release ();
//
//    ++count;
//  } // end WHILE
//  if (result != MF_E_NO_MORE_TYPES)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", returning\n"),
//                count,
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return;
//  } // end IF
//}
bool
Stream_Module_Device_MediaFoundation_Tools::expand (TOPOLOGY_PATH_T& path_inout,
                                                    TOPOLOGY_PATHS_T& paths_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::expand"));

  TOPOLOGY_PATH_ITERATOR_T iterator = path_inout.begin ();
  for (;
       iterator != path_inout.end ();
       ++iterator);
  --iterator;

  HRESULT result = E_FAIL;
  DWORD number_of_outputs = 0;
  result = (*iterator)->GetOutputCount (&number_of_outputs);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_outputs == 0)
    return false; // done (no changes)

  IMFTopologyNode* topology_node_p = NULL;
  DWORD input_index = 0;
  TOPOLOGY_PATH_T topology_path;
  for (DWORD i = 0;
       i < number_of_outputs;
       ++i)
  {
    topology_node_p = NULL;
    result = (*iterator)->GetOutput (i,
                                     &topology_node_p,
                                     &input_index);
    ACE_ASSERT (SUCCEEDED (result));

    topology_path = path_inout;
    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = topology_path.begin ();
         iterator_2 != topology_path.end ();
         ++iterator_2)
      (*iterator_2)->AddRef ();
    topology_path.push_back (topology_node_p);
    paths_inout.push_back (topology_path);
  } // end FOR

  return true;
}
void
Stream_Module_Device_MediaFoundation_Tools::dump (IMFTopology* IMFTopology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  HRESULT result = S_OK;
  WORD count = 0;
  result = IMFTopology_in->GetNodeCount (&count);
  ACE_ASSERT (SUCCEEDED (result));
  TOPOID id = 0;
  result = IMFTopology_in->GetTopologyID (&id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("topology (id: %q, %u nodes)\n"),
              id,
              count));
  IMFTopologyNode* topology_node_p = NULL;
  MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  //for (WORD i = 0;
  //     i < count;
  //     ++i)
  //{
  //  topology_node_p = NULL;
  //  result = IMFTopology_in->GetNode (i,
  //                                    &topology_node_p);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  result = topology_node_p->GetNodeType (&node_type);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  result = topology_node_p->GetTopoNodeID (&id);
  //  ACE_ASSERT (SUCCEEDED (result));

  //  ACE_DEBUG ((LM_INFO,
  //              ACE_TEXT ("#%u: %s (id: %q)\n"),
  //              i + 1,
  //              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::nodeTypeToString (node_type).c_str ()),
  //              id));

  //  topology_node_p->Release ();
  //} // end FOR
  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("topology contains no source nodes, done\n")));

    // clean up
    collection_p->Release ();

    return;
  } // end IF
  TOPOLOGY_PATHS_T topology_paths;
  TOPOLOGY_PATH_T topology_path;
  IUnknown* unknown_p = NULL;
  for (DWORD i = 0;
       i < number_of_source_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    topology_node_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();
      collection_p->Release ();

      return;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_p);
    topology_path.clear ();
    topology_path.push_back (topology_node_p);
    topology_paths.push_back (topology_path);
  } // end FOR
  collection_p->Release ();

  bool changed = false;
  do
  {
    changed = false;
    for (TOPOLOGY_PATHS_ITERATOR_T iterator = topology_paths.begin ();
         iterator != topology_paths.end ();
         ++iterator)
    {
      if (Stream_Module_Device_MediaFoundation_Tools::expand (*iterator,
                                              topology_paths))
      {
        for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
             iterator_2 != (*iterator).end ();
             ++iterator_2)
          (*iterator_2)->Release ();
        topology_paths.erase (iterator);
        changed = true;
        break;
      } // end IF
    } // end FOR
    if (!changed)
      break;
  } while (true);

  std::string topology_string_base, topology_string;
  std::ostringstream converter;
  int index = 0;
  for (TOPOLOGY_PATHS_ITERATOR_T iterator = topology_paths.begin ();
       iterator != topology_paths.end ();
       ++iterator, ++index)
  {
    topology_string_base = ACE_TEXT_ALWAYS_CHAR ("#");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << index + 1;
    topology_string_base += converter.str ();
    topology_string_base += ACE_TEXT_ALWAYS_CHAR (": ");

    TOPOLOGY_PATH_ITERATOR_T iterator_3;
    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
         iterator_2 != (*iterator).end ();
         ++iterator_2)
    {
      result = (*iterator_2)->GetNodeType (&node_type);
      ACE_ASSERT (SUCCEEDED (result));
      result = (*iterator_2)->GetTopoNodeID (&id);
      ACE_ASSERT (SUCCEEDED (result));

      topology_string =
        Stream_Module_Device_MediaFoundation_Tools::nodeTypeToString (node_type);
      topology_string += ACE_TEXT_ALWAYS_CHAR (" (");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << id;
      topology_string += converter.str ();
      topology_string += ACE_TEXT_ALWAYS_CHAR (")");

      iterator_3 = iterator_2;
      if (++iterator_3 != (*iterator).end ())
        topology_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

      topology_string_base += topology_string;

      // clean up
      (*iterator_2)->Release ();
    } // end FOR
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s\n"),
                ACE_TEXT (topology_string_base.c_str ())));
  } // end FOR
}
void
Stream_Module_Device_MediaFoundation_Tools::dump (IMFTransform* IMFTransform_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);

  HRESULT result = S_OK;
  DWORD number_of_input_streams = 0;
  DWORD number_of_output_streams = 0;
  result =
    IMFTransform_in->GetStreamCount (&number_of_input_streams,
                                     &number_of_output_streams);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetStreamCount(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  DWORD* input_stream_ids_p = NULL;
  ACE_NEW_NORETURN (input_stream_ids_p,
                    DWORD [number_of_input_streams]);
  DWORD* output_stream_ids_p = NULL;
  ACE_NEW_NORETURN (output_stream_ids_p,
                    DWORD [number_of_output_streams]);
  if (!input_stream_ids_p || !output_stream_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));

    // clean up
    if (input_stream_ids_p)
      delete [] input_stream_ids_p;
    if (output_stream_ids_p)
      delete[] output_stream_ids_p;

    return;
  } // end IF
  result =
    IMFTransform_in->GetStreamIDs (number_of_input_streams,
                                   input_stream_ids_p,
                                   number_of_output_streams,
                                   output_stream_ids_p);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetStreamIDs(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      delete [] input_stream_ids_p;
      delete [] output_stream_ids_p;

      return;
    } // end IF

    int i = 0;
    for (;
         i < static_cast<int> (number_of_input_streams);
         ++i)
      input_stream_ids_p[i] = i;
    for (i = 0;
         i < static_cast<int> (number_of_output_streams);
         ++i)
      output_stream_ids_p[i] = i;
  } // end IF
  delete [] input_stream_ids_p;
  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  for (int i = 0;
       i < static_cast<int> (number_of_output_streams);
       ++i)
  {
    count = 0;

    while (true)
    {
      media_type_p = NULL;
      result =
        IMFTransform_in->GetOutputAvailableType (output_stream_ids_p[i],
                                                 count,
                                                 &media_type_p);
      if (FAILED (result)) break; // MF_E_TRANSFORM_TYPE_NOT_SET: 0xC00D6D60L

      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("#%d: %s\n"),
                  count + 1,
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (media_type_p).c_str ())));

      // clean up
      media_type_p->Release ();

      ++count;
    } // end WHILE
  } // end FOR
  delete [] output_stream_ids_p;
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetOutputAvailableType(%d): \"%s\", returning\n"),
                count,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
}

//bool
//Stream_Module_Device_MediaFoundation_Tools::getSourceReader (IMFMediaSource*& mediaSource_inout,
//                                             WCHAR*& symbolicLink_out,
//                                             UINT32& symbolicLinkSize_out,
//                                             const IDirect3DDeviceManager9* IDirect3DDeviceManager9_in,
//                                             const IMFSourceReaderCallback* callback_in,
//                                             bool isChromaLuminance_in, 
//                                             IMFSourceReaderEx*& sourceReader_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getSourceReader"));
//
//  bool result = false;
//
//  if (symbolicLinkSize_out)
//  {
//    // sanity check(s)
//    ACE_ASSERT (symbolicLink_out);
//
//    CoTaskMemFree (symbolicLink_out);
//    symbolicLink_out = NULL;
//    symbolicLinkSize_out = 0;
//  } // end IF
//  if (sourceReader_out)
//  {
//    sourceReader_out->Release ();
//    sourceReader_out = NULL;
//  } // end IF
//
//  if (!mediaSource_inout)
//    if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (std::string (),
//                                                     mediaSource_inout,
//                                                     symbolicLink_out,
//                                                     symbolicLinkSize_out))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(), aborting\n")));
//      return false;
//    } // end IF
//  ACE_ASSERT (mediaSource_inout);
//
//  IMFAttributes* attributes_p = NULL;
//  HRESULT result_2 = MFCreateAttributes (&attributes_p, 10);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    return false;
//  } // end IF
//
//  IUnknown* iunknown_p = NULL;
//  //result_2 = attributes_p->SetUINT32 (MF_READWRITE_DISABLE_CONVERTERS,
//  //                                    FALSE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_READWRITE_DISABLE_CONVERTERS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  if (IDirect3DDeviceManager9_in)
//  {
//    result_2 = attributes_p->SetUINT32 (MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS,
//                                        TRUE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  if (callback_in)
//  {
//    iunknown_p = const_cast<IMFSourceReaderCallback*> (callback_in);
//    result_2 =
//      attributes_p->SetUnknown (MF_SOURCE_READER_ASYNC_CALLBACK,
//                                iunknown_p);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUnknown(): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  if (IDirect3DDeviceManager9_in)
//  {
//    iunknown_p =
//      const_cast<IDirect3DDeviceManager9*> (IDirect3DDeviceManager9_in);
//    result_2 =
//      attributes_p->SetUnknown (MF_SOURCE_READER_D3D_MANAGER,
//                                iunknown_p);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUnknown(MF_SOURCE_READER_D3D_MANAGER): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//
//    result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISABLE_DXVA,
//                                        FALSE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISABLE_DXVA): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  //result_2 = attributes_p->SetUnknown (MF_SOURCE_READER_MEDIASOURCE_CONFIG,
//  //                                     NULL); // IPropertyBag handle
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_MEDIASOURCE_CONFIG): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS,
//  //                                    TRUE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  // *NOTE*: allow conversion from YUV to RGB-32 ?
//  // *NOTE*: "...Avoid this setting if you are using Direct3D to display the
//  //         video frames, because the GPU generally provides better video
//  //         processing capabilities.
//  //         If this attribute is TRUE, the following attributes must be FALSE :
//  //         • MF_SOURCE_READER_D3D_MANAGER
//  //         • MF_READWRITE_DISABLE_CONVERTERS ..."
//  if (!IDirect3DDeviceManager9_in)
//  {
//    if (isChromaLuminance_in)
//    {
//      result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING,
//                                          TRUE);
//      if (FAILED (result_2))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING): \"%s\", aborting\n"),
//                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//        goto error;
//      } // end IF
//    } // end IF
//  } // end IF
//  else
//  {
//    // *NOTE*: "... If this attribute is TRUE, the
//    //         MF_READWRITE_DISABLE_CONVERTERS attribute must be FALSE. ..."
//    result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING,
//                                        TRUE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISABLE_CAMERA_PLUGINS,
//                                      TRUE);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISABLE_CAMERA_PLUGINS): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN,
//  //                                    TRUE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS,
//  //                                    FALSE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  //result_2 = attributes_p->SetUnknown (MFT_FIELDOFUSE_UNLOCK_Attribute,
//  //                                     NULL); // IMFFieldOfUseMFTUnlock handle
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUnknown(MFT_FIELDOFUSE_UNLOCK_Attribute): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  IMFSourceReader* source_reader_p = NULL;
//  result_2 = MFCreateSourceReaderFromMediaSource (mediaSource_inout,
//                                                  attributes_p,
//                                                  &source_reader_p);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateSourceReaderFromMediaSource(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//  result_2 = source_reader_p->QueryInterface (IID_PPV_ARGS (&sourceReader_out));
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::QueryInterface(IID_IMFSourceReaderEx): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//
//    // clean up
//    source_reader_p->Release ();
//
//    goto error;
//  } // end IF
//  source_reader_p->Release ();
//
//  result = true;
//
//error:
//  if (attributes_p)
//    attributes_p->Release ();
//
//  return result;
//}
bool
Stream_Module_Device_MediaFoundation_Tools::getMediaSource (const std::string& deviceName_in,
                                                            REFGUID deviceCategory_in,
                                                            IMFMediaSource*& mediaSource_out,
                                                            WCHAR*& symbolicLink_out,
                                                            UINT32& symbolicLinkSize_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getDeviceHandle"));

  bool result = false;

  if (mediaSource_out)
  {
    mediaSource_out->Release ();
    mediaSource_out = NULL;
  } // end IF
  if (symbolicLinkSize_out)
  {
    // sanity check(s)
    ACE_ASSERT (symbolicLink_out);

    CoTaskMemFree (symbolicLink_out);
    symbolicLink_out = NULL;
    symbolicLinkSize_out = 0;
  } // end IF

  IMFAttributes* attributes_p = NULL;
  UINT32 count = 0;
  IMFActivate** devices_pp = NULL;
  unsigned int index = 0;

  HRESULT result_2 = MFCreateAttributes (&attributes_p, 1);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return false;
  } // end IF

  result_2 =
    attributes_p->SetGUID (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                           deviceCategory_in);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::SetGUID(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF

  result_2 = MFEnumDeviceSources (attributes_p,
                                  &devices_pp,
                                  &count);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (devices_pp);
  if (count == 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no capture devices found, aborting\n")));
    goto error;
  } // end IF

  if (!deviceName_in.empty ())
  {
    WCHAR buffer[BUFSIZ];
    UINT32 length;
    bool found = false;
    for (UINT32 i = 0; i < count; i++)
    {
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      length = 0;
      result_2 =
        devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                      buffer,
                                      sizeof (buffer),
                                      &length);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      if (ACE_OS::strcmp (buffer,
                          ACE_TEXT_ALWAYS_WCHAR (deviceName_in.c_str ())) == 0)
      {
        found = true;
        index = i;
        break;
      } // end IF
    } // end FOR
    if (!found)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("capture device (was: \"%s\") not found, aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      goto error;
    } // end IF
  } // end IF
  result_2 =
    devices_pp[index]->ActivateObject (IID_PPV_ARGS (&mediaSource_out));
  if (FAILED (result_2)) // MF_E_SHUTDOWN: 0xC00D3E85
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF

  struct _GUID link_property_id = GUID_NULL;
  if (deviceCategory_in == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
    //link_property_id = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK;
    link_property_id = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID;
  else if (deviceCategory_in == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
    link_property_id = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category, aborting\n")));
    goto error;
  } // end IF
  result_2 =
    devices_pp[index]->GetAllocatedString (link_property_id,
                                           &symbolicLink_out,
                                           &symbolicLinkSize_out);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::GetAllocatedString(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF

  result = true;

error:
  if (attributes_p)
    attributes_p->Release ();

  for (UINT32 i = 0; i < count; i++)
    devices_pp[i]->Release ();
  CoTaskMemFree (devices_pp);

  if (!result && mediaSource_out)
  {
    mediaSource_out->Release ();
    mediaSource_out = NULL;
  } // end IF
  if (!result && symbolicLink_out)
  {
    CoTaskMemFree (symbolicLink_out);
    symbolicLink_out = NULL;
  } // end IF
  if (!result)
    symbolicLinkSize_out = 0;

  return result;
}
bool
Stream_Module_Device_MediaFoundation_Tools::getMediaSource (const IMFMediaSession* IMFMediaSession_in,
                                                            IMFMediaSource*& IMFMediaSource_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getMediaSource"));

  // initialize return value(s)
  if (IMFMediaSource_out)
  {
    IMFMediaSource_out->Release ();
    IMFMediaSource_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IMFMediaSession_in);

  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFTopology* topology_p = NULL;
  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
  //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
  //         --> (try to) wait for the next MESessionTopologySet event
  // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
  //         still fails with MF_E_INVALIDREQUEST)
  HRESULT result = E_FAIL;
  do
  {
    result =
      const_cast<IMFMediaSession*> (IMFMediaSession_in)->GetFullTopology (flags,
                                                                          0,
                                                                          &topology_p);
  } while (result == MF_E_INVALIDREQUEST);
  if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (topology_p,
                                                   IMFMediaSource_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(), aborting\n")));

    // clean up
    topology_p->Release ();

    return false;
  } // end IF
  topology_p->Release ();
  ACE_ASSERT (IMFMediaSource_out);

  return true;
}
bool
Stream_Module_Device_MediaFoundation_Tools::getMediaSource (const IMFTopology* IMFTopology_in,
                                                            IMFMediaSource*& IMFMediaSource_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getMediaSource"));

  // initialize return value(s)
  if (IMFMediaSource_out)
  {
    IMFMediaSource_out->Release ();
    IMFMediaSource_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  IMFCollection* collection_p = NULL;
  HRESULT result =
    const_cast<IMFTopology*> (IMFTopology_in)->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("media session topology has no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();

    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();

    return false;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;
  result = topology_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&IMFMediaSource_out));
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release ();

  return true;
}

bool
Stream_Module_Device_MediaFoundation_Tools::getSampleGrabberNodeId (const IMFTopology* IMFTopology_in,
                                                                    TOPOID& nodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getSampleGrabberNodeId"));

  // initialize return value(s)
  nodeId_out = 0;

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  IMFCollection* collection_p = NULL;
  HRESULT result =
    const_cast<IMFTopology*> (IMFTopology_in)->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_output_nodes = 0;
  result = collection_p->GetElementCount (&number_of_output_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_output_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("media session topology has no sink nodes, aborting\n")));

    // clean up
    collection_p->Release ();

    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();

    return false;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;
  result = topology_node_p->GetTopoNodeID (&nodeId_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::GetTopoNodeID(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_p->Release ();

    return false;
  } // end IF
  topology_node_p->Release ();

  return true;
}

bool
Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology (const std::string& deviceName_in,
                                                                const struct _GUID& deviceCategory_in,
                                                                IMFMediaSource*& IMFMediaSource_inout,
                                                                const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                                IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology"));

  // initialize return value(s)
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

  IMFTopologyNode* topology_node_p = NULL;
  IMFTopologyNode* topology_node_2 = NULL;
  TOPOID node_id = 0;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  WCHAR* symbolic_link_p = NULL;
  UINT32 symbolic_link_size = 0;
  BOOL is_selected = FALSE;
  IMFMediaType* media_type_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFActivate* activate_p = NULL;

  HRESULT result = MFCreateTopology (&IMFTopology_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (deviceCategory_in == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
  {
    result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                         MFTOPOLOGY_DXVA_FULL);
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                       MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                       FALSE);
  ACE_ASSERT (SUCCEEDED (result));

  result = MFCreateTopologyNode (MF_TOPOLOGY_SOURCESTREAM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  if (!IMFMediaSource_inout)
  {
    if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (deviceName_in,
                                                     deviceCategory_in,
                                                     IMFMediaSource_inout,
                                                     symbolic_link_p,
                                                     symbolic_link_size))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      goto error;
    } // end IF
    if (!deviceName_in.empty ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s --> %s\n"),
                  ACE_TEXT (deviceName_in.c_str ()),
                  ACE_TEXT_WCHAR_TO_TCHAR (symbolic_link_p)));
    CoTaskMemFree (symbolic_link_p);
  } // end IF
  ACE_ASSERT (IMFMediaSource_inout);
  result = topology_node_p->SetUnknown (MF_TOPONODE_SOURCE,
                                        IMFMediaSource_inout);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    IMFMediaSource_inout->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result =
    topology_node_p->SetUnknown (MF_TOPONODE_PRESENTATION_DESCRIPTOR,
                                 presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  result = topology_node_p->SetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                        stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  result = IMFTopology_out->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added source node (id: %q)...\n"),
              node_id));

  // *NOTE*: add 'dummy' sink node so the topology can be loaded ?
  if (!IMFSampleGrabberSinkCallback2_in)
    goto continue_;

  if (!Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (IMFMediaSource_inout,
                                                     media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF

  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,
                                       const_cast<IMFSampleGrabberSinkCallback2*> (IMFSampleGrabberSinkCallback2_in),
                                       &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    media_type_p->Release ();

    goto error;
  } // end IF
  media_type_p->Release ();
  // To run as fast as possible, set this attribute (requires Windows 7):
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_2->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_2->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_2->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_2->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->AddNode (topology_node_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_2->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added 'dummy' sink node (id: %q)...\n"),
              node_id));
  result = topology_node_p->ConnectOutput (0,
                                           topology_node_2,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_2->Release ();
  topology_node_2 = NULL;
continue_:
  topology_node_p->Release ();
  topology_node_p = NULL;

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (topology_node_2)
    topology_node_2->Release ();
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::loadSourceTopology (const std::string& URL_in,
                                                                IMFMediaSource*& IMFMediaSource_inout,
                                                                IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::loadSourceTopology"));

  // initialize return value(s)
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

  bool release_source = false;
  IMFTopologyNode* topology_node_p = NULL;
  IMFTopologyNode* topology_node_2 = NULL;
  TOPOID node_id = 0;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  IMFMediaType* media_type_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFActivate* activate_p = NULL;

  HRESULT result = MFCreateTopology (&IMFTopology_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                       MFTOPOLOGY_DXVA_FULL);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                       FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                       MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                       FALSE);
  ACE_ASSERT (SUCCEEDED (result));

  result = MFCreateTopologyNode (MF_TOPOLOGY_SOURCESTREAM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  if (!IMFMediaSource_inout)
  {
    IMFSourceResolver* source_resolver_p = NULL;
    result = MFCreateSourceResolver (&source_resolver_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateSourceResolver(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    DWORD flags = MF_RESOLUTION_MEDIASOURCE;
    IPropertyStore* properties_p = NULL;
    enum MF_OBJECT_TYPE object_type = MF_OBJECT_INVALID;
    IUnknown* unknown_p = NULL;
    result =
      source_resolver_p->CreateObjectFromURL (ACE_TEXT_ALWAYS_WCHAR (URL_in.c_str ()),
                                              flags,
                                              properties_p,
                                              &object_type,
                                              &unknown_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFSourceResolver::CreateObjectFromURL(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (URL_in.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      source_resolver_p->Release ();

      goto error;
    } // end IF
    ACE_ASSERT (unknown_p);
    ACE_ASSERT (object_type = MF_OBJECT_MEDIASOURCE);
    source_resolver_p->Release ();
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&IMFMediaSource_inout));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFMediaSource): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();
    release_source = true;
  } // end IF
  ACE_ASSERT (IMFMediaSource_inout);
  result = topology_node_p->SetUnknown (MF_TOPONODE_SOURCE,
                                        IMFMediaSource_inout);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    IMFMediaSource_inout->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result =
    topology_node_p->SetUnknown (MF_TOPONODE_PRESENTATION_DESCRIPTOR,
                                 presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  result = topology_node_p->SetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                        stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  result = IMFTopology_out->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added source node (id: %q)...\n"),
              node_id));

  topology_node_p->Release ();
  topology_node_p = NULL;

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (topology_node_2)
    topology_node_2->Release ();
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (release_source)
  {
    IMFMediaSource_inout->Release ();
    IMFMediaSource_inout = NULL;
  } // end IF
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::loadSourceTopology (IMFMediaSource* IMFMediaSource_in,
                                                                IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::loadSourceTopology"));

  // initialize return value(s)
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

  IMFTopologyNode* topology_node_p = NULL;
  TOPOID node_id = 0;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = false;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  IMFMediaType* media_type_p = NULL;
  struct _GUID sub_type = GUID_NULL;

  HRESULT result = MFCreateTopology (&IMFTopology_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                       MFTOPOLOGY_DXVA_FULL);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                       FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                       MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_out->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                       FALSE);
  ACE_ASSERT (SUCCEEDED (result));

  result = MFCreateTopologyNode (MF_TOPOLOGY_SOURCESTREAM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (IMFMediaSource_in);
  result = topology_node_p->SetUnknown (MF_TOPONODE_SOURCE,
                                        IMFMediaSource_in);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result =
    topology_node_p->SetUnknown (MF_TOPONODE_PRESENTATION_DESCRIPTOR,
                                 presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  result = topology_node_p->SetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                        stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));

  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;
  result = media_type_handler_p->GetCurrentMediaType (&media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release ();
  media_type_handler_p = NULL;
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_p->Release ();
  media_type_p = NULL;

  result = IMFTopology_out->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added source: \"%s\" -->...\n"),
              node_id,
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));

  topology_node_p->Release ();
  topology_node_p = NULL;

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration (IMFTopology* IMFTopology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  // step1: find a(n output) node that supports the Direct3D manager (typically:
  //        EVR)
  IDirect3DDeviceManager9* direct3D_device_manager_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = IMFTopology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_output_nodes = 0;
  result = collection_p->GetElementCount (&number_of_output_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  TOPOID node_id = 0;
  for (DWORD i = 0;
       i < number_of_output_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    topology_node_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();
      collection_p->Release ();

      return false;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_p);

    result = MFGetService (topology_node_p,
                           MR_VIDEO_ACCELERATION_SERVICE,
                           IID_PPV_ARGS (&direct3D_device_manager_p));
    if (SUCCEEDED (result))
    {
      result = topology_node_p->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("node (id was: %q) supports MR_VIDEO_ACCELERATION_SERVICE...\n"),
                  node_id));
      break;
    } // end IF

    topology_node_p->Release ();
  } // end FOR
  collection_p->Release ();
  topology_node_p->Release ();
  if (!direct3D_device_manager_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no topology node supports MR_VIDEO_ACCELERATION_SERVICE, aborting\n")));
    return false;
  } // end IF

  // step2: iterate over all transfrom nodes and activate DirectX acceleration
  WORD count = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  IMFTransform* transform_p = NULL;
  IMFAttributes* attributes_p = NULL;
  UINT32 is_Direct3D_aware = 0;
  ULONG_PTR pointer_p = reinterpret_cast<ULONG_PTR> (direct3D_device_manager_p);
  result = IMFTopology_in->GetNodeCount (&count);
  ACE_ASSERT (SUCCEEDED (result));
  for (WORD i = 0;
       i < count;
       ++i)
  {
    topology_node_p = NULL;
    result = IMFTopology_in->GetNode (i, &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->GetNodeType (&node_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_type != MF_TOPOLOGY_TRANSFORM_NODE)
    {
      topology_node_p->Release ();
      continue;
    } // end IF

    unknown_p = NULL;
    result = topology_node_p->GetObject (&unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    transform_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();
      topology_node_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();

    result = transform_p->GetAttributes (&attributes_p);
    ACE_ASSERT (SUCCEEDED (result));
    is_Direct3D_aware = MFGetAttributeUINT32 (attributes_p,
                                              MF_SA_D3D_AWARE,
                                              FALSE);
    if (!is_Direct3D_aware)
    {
      // clean up
      attributes_p->Release ();
      transform_p->Release ();
      topology_node_p->Release ();

      continue;
    } // end IF
    attributes_p->Release ();

    result = transform_p->ProcessMessage (MFT_MESSAGE_SET_D3D_MANAGER,
                                          pointer_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();
      topology_node_p->Release ();

      goto error;
    } // end IF
    transform_p->Release ();

    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE, TRUE);
    ACE_ASSERT (SUCCEEDED (result));

    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("node (id was: %q) enabled MR_VIDEO_ACCELERATION_SERVICE...\n"),
                node_id));

    topology_node_p->Release ();
  } // end FOR
  direct3D_device_manager_p->Release ();

  return true;

error:
  if (direct3D_device_manager_p)
    direct3D_device_manager_p->Release ();

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::addGrabber (const IMFMediaType* IMFMediaType_in,
                                                        const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                        IMFTopology* IMFTopology_in,
                                                        TOPOID& grabberNodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::addGrabber"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaType_in);
  ACE_ASSERT (IMFSampleGrabberSinkCallback2_in);
  ACE_ASSERT (IMFTopology_in);

  // initialize return value(s)
  grabberNodeId_out = 0;

  // step1: create sample grabber sink
  IMFActivate* activate_p = NULL;
  HRESULT result =
    MFCreateSampleGrabberSinkActivate (const_cast<IMFMediaType*> (IMFMediaType_in),
                                       const_cast<IMFSampleGrabberSinkCallback2*> (IMFSampleGrabberSinkCallback2_in),
                                       &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    activate_p->Release ();

    return false;
  } // end IF
  activate_p->Release ();

  IMFStreamSink* stream_sink_p = NULL;

  // step2: add node to topology
  IMFTopologyNode* topology_node_p = NULL;
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_in->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&grabberNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added grabber node (id: %q)...\n"),
              grabberNodeId_out));
  topology_node_p->Release ();
  topology_node_p = NULL;

  if (!Stream_Module_Device_MediaFoundation_Tools::append (IMFTopology_in,
                                           grabberNodeId_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::append(%q), aborting\n"),
                grabberNodeId_out));
    goto error;
  } // end IF

  return true;

error:
  if (media_sink_p)
    media_sink_p->Release ();
  if (grabberNodeId_out)
  {
    if (topology_node_p)
    {
      topology_node_p->Release ();
      topology_node_p = NULL;
    } // end IF
    result = IMFTopology_in->GetNodeByID (grabberNodeId_out,
                                          &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_in->RemoveNode (topology_node_p);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::RemoveNode(%q): \"%s\", continuing\n"),
                  grabberNodeId_out,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    grabberNodeId_out = 0;
  } // end IF
  if (topology_node_p)
    topology_node_p->Release ();

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::addRenderer (const HWND windowHandle_in,
                                                         IMFTopology* IMFTopology_in,
                                                         TOPOID& rendererNodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::addRenderer"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  // initialize return value(s)
  rendererNodeId_out = 0;

  IMFTopologyNode* topology_node_p = NULL;

  // step1: create (EVR) renderer
  IMFActivate* activate_p = NULL;
  HRESULT result = MFCreateVideoRendererActivate (windowHandle_in,
                                                  &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  activate_p->Release ();

  //  return false;
  //} // end IF
  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    activate_p->Release ();

    return false;
  } // end IF
  activate_p->Release ();

  //result = MFCreateVideoRenderer (IID_PPV_ARGS (&media_sink_p));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoRenderer(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  IMFPresentationTimeSource* presentation_time_source_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;

  IMFPresentationClock* presentation_clock_p = NULL;
  result = MFCreatePresentationClock (&presentation_clock_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreatePresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = MFCreateSystemTimeSource (&presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSystemTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  result = presentation_clock_p->SetTimeSource (presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::SetTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_time_source_p->Release ();
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  presentation_time_source_p->Release ();
  result = media_sink_p->SetPresentationClock (presentation_clock_p);
  if (FAILED (result)) // MF_E_NOT_INITIALIZED: 0xC00D36B6L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::SetPresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  result = presentation_clock_p->Start (0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::Start(0): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  presentation_clock_p->Release ();

  //IMFTransform* transform_p = NULL;
  //result =
  //  MFCreateVideoMixer (NULL,                         // owner
  //                      IID_IDirect3DDevice9,         // device
  //                      IID_PPV_ARGS (&transform_p)); // return value: interface handle
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  // step2: add node to topology
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_in->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
              rendererNodeId_out));
  topology_node_p->Release ();
  topology_node_p = NULL;

  if (!Stream_Module_Device_MediaFoundation_Tools::append (IMFTopology_in,
                                           rendererNodeId_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::append(%q), aborting\n"),
                rendererNodeId_out));
    goto error;
  } // end IF

  if (!Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration (IMFTopology_in))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration(), continuing\n")));

  return true;

error:
  if (media_sink_p)
    media_sink_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (rendererNodeId_out)
  {
    topology_node_p = NULL;
    result = IMFTopology_in->GetNodeByID (rendererNodeId_out,
                                          &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_in->RemoveNode (topology_node_p);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::RemoveNode(%q): \"%s\", continuing\n"),
                  rendererNodeId_out,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    // *TODO*: apparently, topology_node_p is invalid after RemoveNode()
    //topology_node_p->Release ();
    rendererNodeId_out = 0;
  } // end IF

  return false;
}

bool
Stream_Module_Device_MediaFoundation_Tools::loadAudioRendererTopology (const std::string& deviceName_in,
                                                                       IMFMediaType* IMFMediaType_inout,
                                                                       const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                                       int audioOutput_in,
                                                                       TOPOID& sampleGrabberSinkNodeId_out,
                                                                       TOPOID& rendererNodeId_out,
                                                                       IMFTopology*& IMFTopology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::loadAudioRendererTopology"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaType_inout);

  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
  IMFMediaSource* media_source_p = NULL;
  IMFMediaType* media_type_p = NULL;
  IMFActivate* activate_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  std::string module_string;
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = E_FAIL;
  DWORD number_of_source_nodes = 0;
  IUnknown* unknown_p = NULL;
  UINT32 item_count = 0;
  IMFActivate** decoders_p = NULL;
  UINT32 number_of_decoders = 0;
  UINT32 flags = (MFT_ENUM_FLAG_SYNCMFT        |
                  MFT_ENUM_FLAG_ASYNCMFT       |
                  MFT_ENUM_FLAG_HARDWARE       |
                  MFT_ENUM_FLAG_FIELDOFUSE     |
                  MFT_ENUM_FLAG_LOCALMFT       |
                  MFT_ENUM_FLAG_TRANSCODE_ONLY |
                  MFT_ENUM_FLAG_SORTANDFILTER);
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
  //IMFAudioProcessorControl* video_processor_control_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;
  if (!IMFTopology_inout)
  {
    if (!Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology (deviceName_in,
                                                         MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                         media_source_p,
                                                         NULL, // do not load a dummy sink
                                                         IMFTopology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      goto error;
    } // end IF
    release_topology = true;
  } // end IF
  else if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (IMFTopology_inout,
                                                        media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(), aborting\n")));
    goto error;
  } // end ELSE IF
  ACE_ASSERT (media_source_p);

  // step1: retrieve source node
  result = IMFTopology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();
    collection_p = NULL;

    goto error;
  } // end IF
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();
    unknown_p = NULL;

    goto error;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;

  // step1a: set default capture media type ?
  result = IMFMediaType_inout->GetCount (&item_count);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (!item_count)
  {
    if (!Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                                       IMFMediaType_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default/preset capture format...\n")));
  } // end IF
  if (!Stream_Module_Device_MediaFoundation_Tools::copyMediaType (IMFMediaType_inout,
                                                                  media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::copyMediaType(), aborting\n")));
    goto error;
  } // end IF
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  // step2: add decoder nodes ?
  mft_register_type_info.guidMajorType = MFMediaType_Audio;
  //BOOL is_compressed = false;
  //result = media_type_p->IsCompressedFormat (&is_compressed);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFMediaType::IsCompressedFormat(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  if (!Stream_Module_Device_Tools::isCompressedAudio (sub_type,
                                                      true))
    goto continue_;

  //if (!media_source_p)
  //{
  //  result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
  //                                      IID_PPV_ARGS (&media_source_p));
  //  ACE_ASSERT (SUCCEEDED (result));
  //} // end IF
  //if (!Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
  //                                                   media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //media_source_p->Release ();
  //media_source_p = NULL;

  //IMFAttributes* attributes_p = NULL;
  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;

    result = MFTEnumEx (MFT_CATEGORY_AUDIO_DECODER, // category
                        flags,                      // flags
                        &mft_register_type_info,    // input type
                        NULL,                       // output type
                        &decoders_p,                // array of decoders
                        &number_of_decoders);       // size of array
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    if (number_of_decoders <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));
      goto error;
    } // end IF

    module_string = Stream_Module_Device_MediaFoundation_Tools::activateToString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));
    for (UINT32 i = 0; i < number_of_decoders; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p);
    //result = transform_p->GetAttributes (&attributes_p);
    //ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    //ACE_ASSERT (SUCCEEDED (result));
    //attributes_p->Release ();
    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF

    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added transform node (id: %q)...\n"),
    //            node_id));
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release ();
    media_type_p = NULL;
    if (!Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (transform_p,
                                                      media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = transform_p->SetOutputType (0,
                                         media_type_p,
                                         0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    transform_p->Release ();
    transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%q: added decoder for \"%s\": \"%s\"...\n"),
                node_id,
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ()),
                ACE_TEXT (module_string.c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (!Stream_Module_Device_Tools::isCompressedAudio (sub_type,
                                                        true))
      break; // done
  } // end WHILE

continue_:
  // step3: add tee node ?
  if ((!IMFSampleGrabberSinkCallback2_in && !(audioOutput_in > 0)) ||
      ((IMFSampleGrabberSinkCallback2_in && !(audioOutput_in > 0)) || // XOR
      (!IMFSampleGrabberSinkCallback2_in &&  (audioOutput_in > 0))))
    goto continue_2;

  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added tee node (id: %q)...\n"),
              node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

continue_2:
  // step4: add sample grabber sink ?
  if (!IMFSampleGrabberSinkCallback2_in)
    goto continue_3;

  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,
                                       const_cast<IMFSampleGrabberSinkCallback2*> (IMFSampleGrabberSinkCallback2_in),
                                       &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  // To run as fast as possible, set this attribute (requires Windows 7):
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  //result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->SetCurrentMediaType (media_type_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&sampleGrabberSinkNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added sample grabber sink node (id: %q)...\n"),
              sampleGrabberSinkNodeId_out));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;

continue_3:
  // step5: add audio renderer sink ?
  if (!(audioOutput_in > 0))
    goto continue_4;

  result = MFCreateAudioRendererActivate (&activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAudioRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  media_type_handler_p = NULL;
  result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->SetCurrentMediaType (media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
              rendererNodeId_out));
  result =
    source_node_p->ConnectOutput ((IMFSampleGrabberSinkCallback2_in ? 1 : 0),
                                  topology_node_p,
                                  0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;
  media_source_p->Release ();
  media_source_p = NULL;
  media_type_p->Release ();
  media_type_p = NULL;
  source_node_p->Release ();
  source_node_p = NULL;

continue_4:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();
  if (release_topology)
  {
    IMFTopology_inout->Release ();
    IMFTopology_inout = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology (const std::string& deviceName_in,
                                                                       const IMFMediaType* IMFMediaType_in,
                                                                       const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                                       const HWND windowHandle_in,
                                                                       TOPOID& sampleGrabberSinkNodeId_out,
                                                                       TOPOID& rendererNodeId_out,
                                                                       IMFTopology*& IMFTopology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology"));

  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
  IMFMediaSource* media_source_p = NULL;
  IMFMediaType* media_type_p = NULL;
  IMFActivate* activate_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  std::string module_string;
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = E_FAIL;
  DWORD number_of_source_nodes = 0;
  IUnknown* unknown_p = NULL;
  UINT32 item_count = 0;
  IMFActivate** decoders_p = NULL;
  UINT32 number_of_decoders = 0;
  UINT32 flags = (MFT_ENUM_FLAG_SYNCMFT        |
                  MFT_ENUM_FLAG_ASYNCMFT       |
                  MFT_ENUM_FLAG_HARDWARE       |
                  MFT_ENUM_FLAG_FIELDOFUSE     |
                  MFT_ENUM_FLAG_LOCALMFT       |
                  MFT_ENUM_FLAG_TRANSCODE_ONLY |
                  MFT_ENUM_FLAG_SORTANDFILTER);
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
  IMFVideoProcessorControl* video_processor_control_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;
  if (!IMFTopology_inout)
  {
    if (!Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology (deviceName_in,
                                                         MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                         media_source_p,
                                                         NULL, // do not load a dummy sink
                                                         IMFTopology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      goto error;
    } // end IF
    release_topology = true;
  } // end IF
  else if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (IMFTopology_inout,
                                                        media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(), aborting\n")));
    goto error;
  } // end ELSE IF
  ACE_ASSERT (media_source_p);

  // step1: retrieve source node
  result = IMFTopology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();
    collection_p = NULL;

    goto error;
  } // end IF
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();
    unknown_p = NULL;

    goto error;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;

  // step1a: set default capture media type ?
  result = const_cast<IMFMediaType*> (IMFMediaType_in)->GetCount (&item_count);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (!item_count)
  {
    if (!Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                                       media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default/preset capture format...\n")));
  } // end IF
  else if (!Stream_Module_Device_MediaFoundation_Tools::copyMediaType (IMFMediaType_in,
                                                                       media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::copyMediaType(), aborting\n")));
    goto error;
  } // end IF
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  // step2: add decoder nodes ?
  mft_register_type_info.guidMajorType = MFMediaType_Video;
  //BOOL is_compressed = false;
  //result = media_type_p->IsCompressedFormat (&is_compressed);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFMediaType::IsCompressedFormat(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  if (!Stream_Module_Device_Tools::isCompressedVideo (sub_type,
                                                      true))
    goto transform;

  //if (!media_source_p)
  //{
  //  result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
  //                                      IID_PPV_ARGS (&media_source_p));
  //  ACE_ASSERT (SUCCEEDED (result));
  //} // end IF
  //if (!Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
  //                                                   media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //media_source_p->Release ();
  //media_source_p = NULL;

  //IMFAttributes* attributes_p = NULL;
  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;

    result = MFTEnumEx (MFT_CATEGORY_VIDEO_DECODER, // category
                        flags,                      // flags
                        &mft_register_type_info,    // input type
                        NULL,                       // output type
                        &decoders_p,                // array of decoders
                        &number_of_decoders);       // size of array
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    if (number_of_decoders <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));
      goto error;
    } // end IF

    module_string = Stream_Module_Device_MediaFoundation_Tools::activateToString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));
    for (UINT32 i = 0; i < number_of_decoders; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p);
    //result = transform_p->GetAttributes (&attributes_p);
    //ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    //ACE_ASSERT (SUCCEEDED (result));
    //attributes_p->Release ();
    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF

    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added transform node (id: %q)...\n"),
    //            node_id));
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release ();
    media_type_p = NULL;
    if (!Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (transform_p,
                                                      media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = transform_p->SetOutputType (0,
                                         media_type_p,
                                         0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    transform_p->Release ();
    transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%q: added decoder for \"%s\": \"%s\"...\n"),
                node_id,
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ()),
                ACE_TEXT (module_string.c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (!Stream_Module_Device_Tools::isCompressedVideo (sub_type,
                                                        true))
      break; // done
  } // end WHILE

transform:
  // transform to RGB ?
  if (Stream_Module_Device_Tools::isRGB (sub_type,
                                         true))
    goto continue_;

  mft_register_type_info.guidSubtype = sub_type;

  decoders_p = NULL;
  number_of_decoders = 0;
  result = MFTEnumEx (MFT_CATEGORY_VIDEO_PROCESSOR, // category
                      flags,                        // flags
                      &mft_register_type_info,      // input type
                      NULL,                         // output type
                      &decoders_p,                  // array of decoders
                      &number_of_decoders);         // size of array
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (number_of_decoders <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot find processor for: \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));
    goto error;
  } // end IF

  module_string = Stream_Module_Device_MediaFoundation_Tools::activateToString (decoders_p[0]);

  result = decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
  ACE_ASSERT (SUCCEEDED (result));
  for (UINT32 i = 0; i < number_of_decoders; i++)
    decoders_p[i]->Release ();
  CoTaskMemFree (decoders_p);
  result = transform_p->SetInputType (0,
                                      media_type_p,
                                      0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF

  result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
  result = topology_node_p->SetObject (transform_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added transform node (id: %q)...\n"),
  //            node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

  while (!Stream_Module_Device_Tools::isRGB (sub_type,
                                             true))
  {
    media_type_p->Release ();
    media_type_p = NULL;
    result = transform_p->GetOutputAvailableType (0,
                                                  i,
                                                  &media_type_p);

    ACE_ASSERT (SUCCEEDED (result));
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    ++i;
  } // end WHILE
  //result = media_type_p->DeleteAllItems ();
  //ACE_ASSERT (SUCCEEDED (result));
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (IMFMediaType_in,
                                             media_type_p,
                                             MF_MT_FRAME_RATE);
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (IMFMediaType_in,
                                             media_type_p,
                                             MF_MT_FRAME_SIZE);
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (IMFMediaType_in,
                                             media_type_p,
                                             MF_MT_INTERLACE_MODE);
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (IMFMediaType_in,
                                             media_type_p,
                                             MF_MT_PIXEL_ASPECT_RATIO);
  result = transform_p->SetOutputType (0,
                                       media_type_p,
                                       0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
#if defined (_DEBUG)
  media_type_p->Release ();
  media_type_p = NULL;
  result = transform_p->GetOutputCurrentType (0,
                                              &media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("output format: \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (media_type_p).c_str ())));
#endif
  result =
    transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
  ACE_ASSERT (SUCCEEDED (result));
  transform_p->Release ();
  transform_p = NULL;
  // *TODO*: (for some unknown reason,) this does nothing...
  result = video_processor_control_p->SetMirror (MIRROR_VERTICAL);
  //result = video_processor_control_p->SetRotation (ROTATION_NORMAL);
  ACE_ASSERT (SUCCEEDED (result));
  video_processor_control_p->Release ();

  // debug info
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added processor for \"%s\": \"%s\"...\n"),
              node_id,
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype).c_str ()),
              ACE_TEXT (module_string.c_str ())));

continue_:
  // step3: add tee node ?
  if ((!IMFSampleGrabberSinkCallback2_in && !windowHandle_in) ||
      ((IMFSampleGrabberSinkCallback2_in && !windowHandle_in) || // XOR
      (!IMFSampleGrabberSinkCallback2_in &&  windowHandle_in)))
    goto continue_2;

  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added tee node (id: %q)...\n"),
              node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

continue_2:
  // step4: add sample grabber sink ?
  if (!IMFSampleGrabberSinkCallback2_in)
    goto continue_3;

  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,
                                       const_cast<IMFSampleGrabberSinkCallback2*> (IMFSampleGrabberSinkCallback2_in),
                                       &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  // To run as fast as possible, set this attribute (requires Windows 7):
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  //result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->SetCurrentMediaType (media_type_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&sampleGrabberSinkNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added sample grabber sink node (id: %q)...\n"),
              sampleGrabberSinkNodeId_out));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;

continue_3:
  // step5: add video renderer sink ?
  if (!windowHandle_in)
    goto continue_4;

  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  media_type_handler_p = NULL;
  result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->SetCurrentMediaType (media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
              rendererNodeId_out));
  result =
    source_node_p->ConnectOutput ((IMFSampleGrabberSinkCallback2_in ? 1 : 0),
                                  topology_node_p,
                                  0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;
  media_source_p->Release ();
  media_source_p = NULL;
  media_type_p->Release ();
  media_type_p = NULL;
  source_node_p->Release ();
  source_node_p = NULL;

  if (!Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration (IMFTopology_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration(), aborting\n")));
    goto error;
  } // end IF

continue_4:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();
  if (release_topology)
  {
    IMFTopology_inout->Release ();
    IMFTopology_inout = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology (const IMFMediaType* mediaType_in,
                                                                       const HWND windowHandle_in,
                                                                       TOPOID& rendererNodeId_out,
                                                                       IMFTopology*& topology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (topology_inout);

  bool release_topology = false;
  struct _GUID sub_type, sub_type_2 = GUID_NULL;
  IMFMediaType* media_type_p = NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
  IMFMediaSource* media_source_p = NULL;
  IMFActivate* activate_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  std::string module_string;
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = E_FAIL;
  DWORD number_of_source_nodes = 0;
  IUnknown* unknown_p = NULL;
  UINT32 item_count = 0;
  IMFActivate** decoders_p = NULL;
  UINT32 number_of_decoders = 0;
  UINT32 flags = (MFT_ENUM_FLAG_SYNCMFT        |
                  MFT_ENUM_FLAG_ASYNCMFT       |
                  MFT_ENUM_FLAG_HARDWARE       |
                  MFT_ENUM_FLAG_FIELDOFUSE     |
                  MFT_ENUM_FLAG_LOCALMFT       |
                  MFT_ENUM_FLAG_TRANSCODE_ONLY |
                  MFT_ENUM_FLAG_SORTANDFILTER);
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
  IMFVideoProcessorControl* video_processor_control_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;
  BOOL is_compressed = false;

  // initialize return value(s)
  rendererNodeId_out = 0;
  
  if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (topology_inout,
                                                   media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getMediaSource(), aborting\n")));
    goto error;
  } // end ELSE IF
  ACE_ASSERT (media_source_p);

  // step1: retrieve source node
  result = topology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();
    collection_p = NULL;

    goto error;
  } // end IF
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();
    unknown_p = NULL;

    goto error;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;

  // step2: (try to-) add decoder nodes ?
  mft_register_type_info.guidMajorType = MFMediaType_Video;
  result =
    const_cast<IMFMediaType*> (mediaType_in)->IsCompressedFormat (&is_compressed);
  ACE_ASSERT (SUCCEEDED (result));
  if (!is_compressed) goto transform;

  result =
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_SUBTYPE,
                                                       &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (!Stream_Module_Device_MediaFoundation_Tools::copyMediaType (mediaType_in,
                                                  media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::copyMediaType(), aborting\n")));
    goto error;
  } // end IF
  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;

    result = MFTEnumEx (MFT_CATEGORY_VIDEO_DECODER, // category
                        flags,                      // flags
                        &mft_register_type_info,    // input type
                        NULL,                       // output type
                        &decoders_p,                // array of decoders
                        &number_of_decoders);       // size of array
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    if (number_of_decoders <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));
      goto error;
    } // end IF

    module_string = Stream_Module_Device_MediaFoundation_Tools::activateToString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));
    for (UINT32 i = 0; i < number_of_decoders; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p);
    //result = transform_p->GetAttributes (&attributes_p);
    //ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    //ACE_ASSERT (SUCCEEDED (result));
    //attributes_p->Release ();
    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF

    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added transform node (id: %q)...\n"),
    //            node_id));
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release ();
    media_type_p = NULL;
    if (!Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (transform_p,
                                                      media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = transform_p->SetOutputType (0,
                                         media_type_p,
                                         0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    transform_p->Release ();
    transform_p = NULL;

    sub_type_2 = sub_type;
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%q: added decoder \"%s\": \"%s\" --> \"%s\"...\n"),
                node_id,
                ACE_TEXT (module_string.c_str ()),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type_2).c_str ()),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));

    result = media_type_p->IsCompressedFormat (&is_compressed);
    ACE_ASSERT (SUCCEEDED (result));
    if (!is_compressed) break; // done
  } // end WHILE

transform:
  // transform to RGB ?
  // *NOTE*: apparently, the default Video Processer MFT from Micrsoft cannot
  //         transform NV12 to RGB (IMFTopoLoder::Load() fails:
  //         MF_E_INVALIDMEDIATYPE). As the EVR can handle most ChromaLuminance
  //         formats directly, skip this step
  if (Stream_Module_Device_Tools::isRGB (sub_type,
                                         true) ||
      Stream_Module_Device_Tools::isChromaLuminance (sub_type,
                                                     true))
    goto continue_;

  mft_register_type_info.guidSubtype = sub_type;

  decoders_p = NULL;
  number_of_decoders = 0;
  result = MFTEnumEx (MFT_CATEGORY_VIDEO_PROCESSOR, // category
                      flags,                        // flags
                      &mft_register_type_info,      // input type
                      NULL,                         // output type
                      &decoders_p,                  // array of decoders
                      &number_of_decoders);         // size of array
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  if (number_of_decoders <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot find processor for: \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));
    goto error;
  } // end IF

  module_string = Stream_Module_Device_MediaFoundation_Tools::activateToString (decoders_p[0]);

  result = decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
  ACE_ASSERT (SUCCEEDED (result));
  for (UINT32 i = 0; i < number_of_decoders; i++)
    decoders_p[i]->Release ();
  CoTaskMemFree (decoders_p);
  result = transform_p->SetInputType (0,
                                      media_type_p,
                                      0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF

  result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
  result = topology_node_p->SetObject (transform_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added transform node (id: %q)...\n"),
  //            node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

  i = 0;
  while (!Stream_Module_Device_Tools::isRGB (sub_type,
                                             true))
  {
    media_type_p->Release ();
    media_type_p = NULL;
    result = transform_p->GetOutputAvailableType (0,
                                                  i,
                                                  &media_type_p);

    ACE_ASSERT (SUCCEEDED (result));
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    ++i;
  } // end WHILE
  //result = media_type_p->DeleteAllItems ();
  //ACE_ASSERT (SUCCEEDED (result));
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (mediaType_in,
                                             media_type_p,
                                             MF_MT_FRAME_RATE);
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (mediaType_in,
                                             media_type_p,
                                             MF_MT_FRAME_SIZE);
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (mediaType_in,
                                             media_type_p,
                                             MF_MT_INTERLACE_MODE);
  Stream_Module_Device_MediaFoundation_Tools::copyAttribute (mediaType_in,
                                             media_type_p,
                                             MF_MT_PIXEL_ASPECT_RATIO);
  result = transform_p->SetOutputType (0,
                                       media_type_p,
                                       0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    transform_p->Release ();

    goto error;
  } // end IF
#if defined (_DEBUG)
  media_type_p->Release ();
  media_type_p = NULL;
  result = transform_p->GetOutputCurrentType (0,
                                              &media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("output format: \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (media_type_p).c_str ())));
#endif
  result =
    transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
  ACE_ASSERT (SUCCEEDED (result));
  transform_p->Release ();
  transform_p = NULL;
  // *TODO*: (for some unknown reason,) this does nothing...
  result = video_processor_control_p->SetMirror (MIRROR_VERTICAL);
  //result = video_processor_control_p->SetRotation (ROTATION_NORMAL);
  ACE_ASSERT (SUCCEEDED (result));
  video_processor_control_p->Release ();

  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  ACE_ASSERT (SUCCEEDED (result));
  // debug info
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added processor \"%s\": \"%s\" --> \"%s\"...\n"),
              node_id,
              ACE_TEXT (module_string.c_str ()),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype).c_str ()),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));

continue_:
  // step5: add video renderer sink ?
  //if (!windowHandle_in) goto continue_2;

  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  // MF_ACTIVATE_CUSTOM_VIDEO_MIXER_ACTIVATE
  // MF_ACTIVATE_CUSTOM_VIDEO_MIXER_CLSID 
  // MF_ACTIVATE_CUSTOM_VIDEO_MIXER_FLAGS 
  // MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_ACTIVATE
  // MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID
  // MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_FLAGS 
  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  module_string = Stream_Module_Device_MediaFoundation_Tools::activateToString (activate_p);

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release ();
  media_sink_p = NULL;
  media_type_handler_p = NULL;
  result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->SetCurrentMediaType (media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release ();

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added renderer \"%s\": \"%s\" -->...\n"),
              rendererNodeId_out,
              ACE_TEXT (module_string.c_str ()),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));

  result =
    source_node_p->ConnectOutput (0,
                                  topology_node_p,
                                  0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;
  media_source_p->Release ();
  media_source_p = NULL;
  media_type_p->Release ();
  media_type_p = NULL;
  source_node_p->Release ();
  source_node_p = NULL;

  if (!Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration (topology_inout))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::enableDirectXAcceleration(), continuing\n")));

//continue_2:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();

  return false;
}

bool
Stream_Module_Device_MediaFoundation_Tools::loadTargetRendererTopology (const std::string& URL_in,
                                                                        const IMFMediaType* IMFMediaType_in,
                                                                        const HWND windowHandle_in,
                                                                        TOPOID& rendererNodeId_out,
                                                                        IMFTopology*& IMFTopology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::loadTargetRendererTopology"));

  IMFMediaSource* media_source_p = NULL;
  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { 0 };
  IMFActivate* activate_p = NULL;

  // initialize return value(s)
  rendererNodeId_out = 0;

  if (!IMFTopology_inout)
  {
    if (!Stream_Module_Device_MediaFoundation_Tools::loadSourceTopology (URL_in,
                                                         media_source_p,
                                                         IMFTopology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::loadSourceTopology(\"%s\"), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (media_source_p);
    release_topology = true;
  } // end IF

  // step1: retrieve source node
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result =
    IMFTopology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();
    collection_p = NULL;

    goto error;
  } // end IF
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();
    unknown_p = NULL;

    goto error;
  } // end IF
  unknown_p->Release ();
  unknown_p = NULL;

  // step2: add decoder nodes ?
  IMFActivate** decoders_p = NULL;
  UINT32 number_of_decoders = 0;
  mft_register_type_info.guidMajorType = MFMediaType_Video;
  UINT32 flags = (MFT_ENUM_FLAG_SYNCMFT        |
                  MFT_ENUM_FLAG_ASYNCMFT       |
                  MFT_ENUM_FLAG_HARDWARE       |
                  MFT_ENUM_FLAG_FIELDOFUSE     |
                  MFT_ENUM_FLAG_LOCALMFT       |
                  MFT_ENUM_FLAG_TRANSCODE_ONLY |
                  MFT_ENUM_FLAG_SORTANDFILTER);
  IMFTransform* transform_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  if (!media_source_p)
  {
    result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&media_source_p));
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  IMFMediaType* media_type_p = NULL;
  if (!Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                     media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
  media_source_p->Release ();
  media_source_p = NULL;
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  if (!Stream_Module_Device_Tools::isCompressedVideo (sub_type,
                                                      true))
    goto continue_;

  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;
    result = MFTEnumEx (MFT_CATEGORY_VIDEO_DECODER, // category
                        flags,                      // flags
                        &mft_register_type_info,    // input type
                        NULL,                       // output type
                        &decoders_p,                // array of decoders
                        &number_of_decoders);       // size of array
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFTEnumEx(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ()),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));
      goto error;
    } // end IF
    if (number_of_decoders <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));
      goto error;
    } // end IF

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));
    for (UINT32 i = 0; i < number_of_decoders; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p);
    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF

    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFTopology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release ();
    media_type_p = NULL;
    result = transform_p->GetOutputCurrentType (0,
                                                &media_type_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      transform_p->Release ();

      goto error;
    } // end IF
    transform_p->Release ();
    transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added decoder for \"%s\"...\n"),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (sub_type).c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    if (!Stream_Module_Device_Tools::isCompressedVideo (sub_type,
                                                        true))
      break; // done
  } // end WHILE
  media_type_p->Release ();
  media_type_p = NULL;

continue_:
  // step5: add video renderer sink ?
  if (!windowHandle_in)
    goto continue_2;

  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->SetObject (activate_p);
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release ();
  activate_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = IMFTopology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release ();
  topology_node_p = NULL;
  source_node_p->Release ();
  source_node_p = NULL;

continue_2:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();
  if (release_topology)
  {
    IMFTopology_inout->Release ();
    IMFTopology_inout = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::setTopology (IMFTopology* IMFTopology_in,
                                                         IMFMediaSession*& IMFMediaSession_inout,
                                                         bool isPartial_in,
                                                         bool waitForCompletion_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::setTopology"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  HRESULT result = E_FAIL;
  bool release_media_session = false;
  IMFTopoLoader* topology_loader_p = NULL;
  IMFTopology* topology_p = NULL;
  DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE    |
                          MFSESSION_SETTOPOLOGY_NORESOLUTION |
                          MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
  if (isPartial_in)
    topology_flags &= ~MFSESSION_SETTOPOLOGY_NORESOLUTION;
  IMFMediaEvent* media_event_p = NULL;
  bool received_topology_set_event = false;
  MediaEventType event_type = MEUnknown;
  ACE_Time_Value timeout (COMMON_UI_WIN32_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT,
                          0);

  // initialize return value(s)
  if (!IMFMediaSession_inout)
  {
    IMFAttributes* attributes_p = NULL;
    result = MFCreateAttributes (&attributes_p, 4);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
    ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
    //ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = MFCreateMediaSession (attributes_p,
                                   &IMFMediaSession_inout);
    if (FAILED (result)) // MF_E_SHUTDOWN: 0xC00D3E85L
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      attributes_p->Release ();

      goto error;
    } // end IF
    attributes_p->Release ();
    release_media_session = true;
  } // end IF
  ACE_ASSERT (IMFMediaSession_inout);

  result = MFCreateTopoLoader (&topology_loader_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopoLoader(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = topology_loader_p->Load (IMFTopology_in,
                                    &topology_p,
                                    NULL);
  if (FAILED (result)) // MF_E_INVALIDMEDIATYPE    : 0xC00D36B4L
  {                    // MF_E_NO_MORE_TYPES       : 0xC00D36B9L
                       // MF_E_TOPO_CODEC_NOT_FOUND: 0xC00D5212L
                       // MF_E_TOPO_UNSUPPORTED:     0xC00D5214L
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopoLoader::Load(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    Stream_Module_Device_MediaFoundation_Tools::dump (IMFTopology_in);
    goto error;
  } // end IF
  topology_loader_p->Release ();
  topology_loader_p = NULL;
  ACE_ASSERT (topology_p);

  result = IMFMediaSession_inout->SetTopology (topology_flags,
                                               topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_p->Release ();
  topology_p = NULL;

  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
  //         to retrieve a topology handle will fail (MF_E_INVALIDREQUEST)
  //         --> wait a little ?
  if (!waitForCompletion_in)
    goto continue_;

  timeout += COMMON_TIME_NOW;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = IMFMediaSession_inout->GetEvent (0,
                                              &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologySet)
      received_topology_set_event = true;
    media_event_p->Release ();
  } while (!received_topology_set_event &&
           (COMMON_TIME_NOW < timeout));
  if (!received_topology_set_event)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): timed out, continuing\n")));

continue_:
  return true;

error:
  if (topology_loader_p)
    topology_loader_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (release_media_session)
  { 
    IMFMediaSession_inout->Release ();
    IMFMediaSession_inout = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Device_MediaFoundation_Tools::append (IMFTopology* IMFTopology_in,
                                                    TOPOID nodeId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::append"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);
  ACE_ASSERT (nodeId_in);

  // step0: retrieve node handle
  bool add_tee_node = true;
  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  result = IMFTopology_in->GetNodeByID (nodeId_in,
                                        &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                nodeId_in,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);

  // step1: find a suitable upstream node
  IMFTopologyNode* topology_node_2 = NULL; // source/output node
  IMFTopologyNode* topology_node_3 = NULL; // upstream node
  IMFMediaType* media_type_p = NULL;
  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_nodes = 0;
  result = collection_p->GetElementCount (&number_of_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  TOPOID node_id = 0;
  DWORD number_of_inputs = 0;
  DWORD output_index = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  IMFTopologyNode* topology_node_4 = NULL;
  DWORD input_index = 0;

  if (number_of_nodes <= 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology contains no output nodes, continuing\n")));

use_source_node:
    add_tee_node = false;
    collection_p->Release ();
    collection_p = NULL;
    result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = collection_p->GetElementCount (&number_of_nodes);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_nodes <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("topology contains no source nodes, aborting\n")));

      // clean up
      collection_p->Release ();

      goto error;
    } // end IF
    result = collection_p->GetElement (0, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    collection_p->Release ();
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_2);

    do
    {
      result = topology_node_2->GetOutputCount (&number_of_nodes);
      ACE_ASSERT (SUCCEEDED (result));
      if (number_of_nodes <= 0) break;

      topology_node_3 = NULL;
      result = topology_node_2->GetOutput (0,
                                           &topology_node_3,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      topology_node_2->Release ();
      topology_node_2 = topology_node_3;
    } while (true);

    goto continue_;
  } // end IF

  for (DWORD i = 0;
       i < number_of_nodes;
       ++i)
  {
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_2);

    result = topology_node_2->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_id == nodeId_in)
    {
      topology_node_2->Release ();
      topology_node_2 = NULL;
      continue;
    } // end IF

    break;
  } // end FOR
  if (!topology_node_2)
    goto use_source_node;
  collection_p->Release ();

  result = topology_node_2->GetInputCount (&number_of_inputs);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (number_of_inputs > 0);
  result = topology_node_2->GetInput (0,
                                      &topology_node_3,
                                      &output_index);
  ACE_ASSERT (SUCCEEDED (result));

continue_:
  ACE_ASSERT (topology_node_p);
  ACE_ASSERT (topology_node_2);
  ACE_ASSERT (topology_node_3);

  result = topology_node_3->GetNodeType (&node_type);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type)
  {
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_3->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result));
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result));
      stream_descriptor_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&media_type_p);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release ();

      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      unknown_p = NULL;
      result = topology_node_3->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        unknown_p->Release ();
        topology_node_2->Release ();
        topology_node_3->Release ();

        goto error;
      } // end IF
      unknown_p->Release ();
      result = transform_p->GetOutputCurrentType (0,
                                                  &media_type_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        transform_p->Release ();
        topology_node_2->Release ();
        topology_node_3->Release ();

        goto error;
      } // end IF
      transform_p->Release ();

      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_3->GetOutputPrefType (0,
                                                   &media_type_p);
      ACE_ASSERT (SUCCEEDED (result));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type));

      // clean up
      topology_node_2->Release ();
      topology_node_3->Release ();

      goto error;
    }
  } // end SWITCH

  // step2: add a tee node ?
  if (!add_tee_node)
  {
    topology_node_4 = topology_node_3;
    topology_node_3 = NULL;
    goto continue_2;
  } // end IF

  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_3->Release ();

    goto error;
  } // end IF
  result = IMFTopology_in->AddNode (topology_node_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_3->Release ();
    topology_node_4->Release ();

    goto error;
  } // end IF
  node_id = 0;
  result = topology_node_4->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added tee node (id: %q)...\n"),
              node_id));

  // step3: connect the upstream node to the tee
  result = topology_node_3->ConnectOutput (0,
                                           topology_node_4,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_3->Release ();
    topology_node_4->Release ();

    goto error;
  } // end IF
  topology_node_3->Release ();
  result = topology_node_4->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (0,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (1,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  // step4: connect the (two) outputs
  result = topology_node_4->ConnectOutput (0,
                                           topology_node_2,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_2->Release ();
    topology_node_4->Release ();

    goto error;
  } // end IF
  result = topology_node_2->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_2->Release ();

continue_2:
  result = topology_node_4->ConnectOutput ((add_tee_node ? 1 : 0),
                                           topology_node_p,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_4->Release ();

    goto error;
  } // end IF
  result = topology_node_p->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_p->Release ();
  topology_node_p->Release ();
  topology_node_4->Release ();

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();

  return false;
}

bool
Stream_Module_Device_MediaFoundation_Tools::clear (IMFMediaSession* IMFMediaSession_in,
                                                   bool waitForCompletion_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::clear"));

  ACE_Time_Value timeout (COMMON_UI_WIN32_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT,
                          0);
  ACE_Time_Value deadline;
  IMFMediaEvent* media_event_p = NULL;
  bool received_topology_event = false;
  MediaEventType event_type = MEUnknown;

  // *NOTE*: this method is asynchronous
  //         --> wait for MESessionTopologiesCleared ?
  HRESULT result = IMFMediaSession_in->ClearTopologies ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::ClearTopologies(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  if (!waitForCompletion_in) goto continue_;

  deadline = COMMON_TIME_NOW + timeout;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = IMFMediaSession_in->GetEvent (0,
                                           &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologiesCleared)
      received_topology_event = true;
    media_event_p->Release ();
  } while (!received_topology_event &&
           (COMMON_TIME_NOW < deadline));
  if (!received_topology_event)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::ClearTopologies(): timed out, continuing\n")));

continue_:
  DWORD topology_flags = MFSESSION_SETTOPOLOGY_CLEAR_CURRENT;
  result = IMFMediaSession_in->SetTopology (topology_flags,
                                            NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(MFSESSION_SETTOPOLOGY_CLEAR_CURRENT): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous
  //         --> wait for the next MESessionTopologySet ?
  if (!waitForCompletion_in)
    return true;

  //deadline = COMMON_TIME_NOW + timeout;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = IMFMediaSession_in->GetEvent (0,
                                           &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologySet)
      received_topology_event = true;
    media_event_p->Release ();
  } while (!received_topology_event &&
           (COMMON_TIME_NOW < deadline));
  if (!received_topology_event)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): timed out, continuing\n")));

  return true;
}
bool
Stream_Module_Device_MediaFoundation_Tools::clear (IMFTopology* IMFTopology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::clear"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);

  HRESULT result = E_FAIL;
  IMFCollection* collection_p = NULL;
  result =
    IMFTopology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    // clean up
    collection_p->Release ();

    return true;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  DWORD number_of_outputs = 0;
  DWORD input_index = 0;
  IMFTopologyNode* topology_node_2 = NULL;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  TOPOID node_id = 0;
  DWORD number_of_outputs_2 = 0;
  IMFTopologyNode* topology_node_3 = NULL;
  for (DWORD i = 0;
       i < number_of_source_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      collection_p->Release ();
      unknown_p->Release ();

      return false;
    } // end IF
    unknown_p->Release ();

    number_of_outputs = 0;
    result = topology_node_p->GetOutputCount (&number_of_outputs);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_outputs <= 0)
    {
      // clean up
      topology_node_p->Release ();

      continue;
    } // end IF
    for (DWORD j = 0;
         j < number_of_outputs;
         ++j)
    {
      topology_node_2 = NULL;
      result = topology_node_p->GetOutput (j,
                                           &topology_node_2,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      result = topology_node_2->GetNodeType (&node_type);
      ACE_ASSERT (SUCCEEDED (result));
      if (node_type != MF_TOPOLOGY_TRANSFORM_NODE)
      {
        // clean up
        topology_node_p->Release ();
        topology_node_2->Release ();

        continue;
      } // end IF

      number_of_outputs_2 = 0;
      result = topology_node_2->GetOutputCount (&number_of_outputs_2);
      ACE_ASSERT (SUCCEEDED (result));
      for (DWORD k = 0;
           k < number_of_outputs_2;
           ++k)
      {
        topology_node_3 = NULL;
        result = topology_node_2->GetOutput (k,
                                             &topology_node_3,
                                             &input_index);
        ACE_ASSERT (SUCCEEDED (result));
        result = topology_node_3->GetTopoNodeID (&node_id);
        ACE_ASSERT (SUCCEEDED (result));
        result = IMFTopology_in->RemoveNode (topology_node_3);
        if (FAILED (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFTopology::RemoveNode(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));

          // clean up
          topology_node_3->Release ();
          topology_node_2->Release ();
          topology_node_p->Release ();
          collection_p->Release ();

          return false;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("removed node (id was: %q)...\n"),
                    node_id));
        topology_node_3->Release ();
      } // end FOR
      result = topology_node_2->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      result = IMFTopology_in->RemoveNode (topology_node_2);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopology::RemoveNode(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        topology_node_2->Release ();
        topology_node_p->Release ();
        collection_p->Release ();

        return false;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("removed transform node (id was: %q)...\n"),
                  node_id));
      topology_node_2->Release ();
    } // end FOR
    topology_node_p->Release ();
  } // end FOR
  collection_p->Release ();

  return true;
}

bool
Stream_Module_Device_MediaFoundation_Tools::disconnect (IMFTopologyNode* IMFTopologyNode_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (IMFTopologyNode_in);

  DWORD number_of_outputs = 0;
  HRESULT result = IMFTopologyNode_in->GetOutputCount (&number_of_outputs);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::GetOutputCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  DWORD input_index = 0;
  for (DWORD i = 0;
       i < number_of_outputs;
       ++i)
  {
    result = IMFTopologyNode_in->GetOutput (i,
                                            &topology_node_p,
                                            &input_index);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::GetOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF

    result = IMFTopologyNode_in->DisconnectOutput (i);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::DisconnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      topology_node_p->Release ();

      return false;
    } // end IF

    if (Stream_Module_Device_MediaFoundation_Tools::disconnect (topology_node_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::disconnect(), aborting\n")));

      // clean up
      topology_node_p->Release ();

      return false;
    } // end IF
    topology_node_p->Release ();
  } // end FOR

  return true;
}

//bool
//Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (IMFSourceReader* sourceReader_in,
//                                              IMFMediaType*& mediaType_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat"));
//
//  // sanity check(s)
//  ACE_ASSERT (sourceReader_in);
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  HRESULT result =
//    sourceReader_in->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                          &mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//  ACE_ASSERT (mediaType_out);
//
//  return true;
//}
//
//bool
//Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (IMFSourceReader* sourceReader_in,
//                                             IMFMediaType*& mediaType_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getOutputFormat"));
//
//  // sanity check(s)
//  ACE_ASSERT (sourceReader_in);
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  HRESULT result = MFCreateMediaType (&mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//
//  result =
//    sourceReader_in->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                          &mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (mediaType_out);
//
//  return true;
//
//error:
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  return false;
//}
bool
Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (IMFMediaSource* IMFMediaSource_in,
                                                              IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaSource_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  BOOL is_selected = FALSE;

  HRESULT result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;
  result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_type_handler_p->Release ();
  media_type_handler_p = NULL;

  return true;

error:
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (IMFTransform* IMFTransform_in,
                                                             IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = S_OK;
  DWORD number_of_input_streams = 0;
  DWORD number_of_output_streams = 0;
  DWORD* input_stream_ids_p = NULL;
  DWORD* output_stream_ids_p = NULL;
  result = IMFTransform_in->GetStreamCount (&number_of_input_streams,
                                            &number_of_output_streams);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetStreamCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_NEW_NORETURN (input_stream_ids_p,
                    DWORD[number_of_input_streams]);
  ACE_NEW_NORETURN (output_stream_ids_p,
                    DWORD[number_of_output_streams]);
  if (!input_stream_ids_p || !output_stream_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    goto error;
  } // end IF
  result = IMFTransform_in->GetStreamIDs (number_of_input_streams,
                                          input_stream_ids_p,
                                          number_of_output_streams,
                                          output_stream_ids_p);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetStreamIDs(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    int i = 0;
    for (;
         i < static_cast<int> (number_of_input_streams);
         ++i)
      input_stream_ids_p[i] = i;
    for (i = 0;
         i < static_cast<int> (number_of_output_streams);
         ++i)
      output_stream_ids_p[i] = i;
  } // end IF
  delete [] input_stream_ids_p;
  input_stream_ids_p = NULL;

  result = IMFTransform_in->GetOutputAvailableType (output_stream_ids_p[0],
                                                    0,
                                                    &IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetOutputAvailableType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  delete [] output_stream_ids_p;

  return true;

error:
  if (input_stream_ids_p)
    delete [] input_stream_ids_p;
  if (output_stream_ids_p)
    delete [] output_stream_ids_p;
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  return false;
}
bool
Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (IMFTopology* IMFTopology_in,
                                                             TOPOID nodeId_in,
                                                             IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFTopology_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  DWORD number_of_nodes = 0;
  IUnknown* unknown_p = NULL;
  IMFTopologyNode* topology_node_2 = NULL;
  DWORD input_index = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;

  if (nodeId_in)
  {
    result = IMFTopology_in->GetNodeByID (nodeId_in,
                                          &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                  nodeId_in,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    goto continue_;
  } // end IF

  result = IMFTopology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_nodes <= 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology contains no output nodes, continuing\n")));
    collection_p->Release ();
    collection_p = NULL;
    result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = collection_p->GetElementCount (&number_of_nodes);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_nodes <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("topology contains no source nodes, aborting\n")));

      // clean up
      collection_p->Release ();

      goto error;
    } // end IF
    result = collection_p->GetElement (0, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    collection_p->Release ();
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();

    do
    {
      result = topology_node_p->GetOutputCount (&number_of_nodes);
      ACE_ASSERT (SUCCEEDED (result));
      if (number_of_nodes <= 0) break;

      topology_node_2 = NULL;
      result = topology_node_p->GetOutput (0,
                                           &topology_node_2,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      topology_node_p->Release ();
      topology_node_p = topology_node_2;
    } while (true);

    goto continue_;
  } // end IF

  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();

    goto error;
  } // end IF
  unknown_p->Release ();

continue_:
  if (!topology_node_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("output node not found [id was: %q], aborting\n"),
                nodeId_in));
    goto error;
  } // end IF

  result = topology_node_p->GetNodeType (&node_type);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_ASSERT (unknown_p);

      IMFStreamSink* stream_sink_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFStreamSink): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        unknown_p->Release ();

        goto error;
      } // end IF
      unknown_p->Release ();
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFStreamSink::GetMediaTypeHandler(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        stream_sink_p->Release ();

        goto error;
      } // end IF
      stream_sink_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        media_type_handler_p->Release ();

        goto error;
      } // end IF
      media_type_handler_p->Release ();
      break;
    }
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_p->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result));
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result));
      stream_descriptor_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release ();
      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_ASSERT (unknown_p);

      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        unknown_p->Release ();

        goto error;
      } // end IF
      unknown_p->Release ();
      result = transform_p->GetOutputCurrentType (0,
                                                  &IMFMediaType_out);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        transform_p->Release ();

        goto error;
      } // end IF
      transform_p->Release ();
      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_p->GetOutputPrefType (0,
                                                   &IMFMediaType_out);
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (IMFMediaType_out);
  topology_node_p->Release ();

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  return false;
}

//bool
//Stream_Module_Device_MediaFoundation_Tools::setOutputFormat (IMFSourceReader* IMFSourceReader_in,
//                                             const IMFMediaType* IMFMediaType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::setOutputFormat"));
//
//  // sanit ycheck(s)
//  ACE_ASSERT (IMFSourceReader_in);
//  ACE_ASSERT (IMFMediaType_in);
//
//  HRESULT result =
//    IMFSourceReader_in->SetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                             NULL,
//                                             const_cast<IMFMediaType*> (IMFMediaType_in));
//  if (FAILED (result)) // MF_E_INVALIDMEDIATYPE: 0xC00D36B4L
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::SetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//
//  return true;
//}

std::string
Stream_Module_Device_MediaFoundation_Tools::nodeTypeToString (enum MF_TOPOLOGY_TYPE nodeType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::nodeTypeToString"));

  std::string result;

  switch (nodeType_in)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("output"); break;
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("source"); break;
    case MF_TOPOLOGY_TRANSFORM_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("transform"); break;
    case MF_TOPOLOGY_TEE_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("tee"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %u), aborting\n"),
                  nodeType_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
Stream_Module_Device_MediaFoundation_Tools::topologyStatusToString (MF_TOPOSTATUS topologyStatus_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::topologyStatusToString"));

  std::string result;

  switch (topologyStatus_in)
  {
    case MF_TOPOSTATUS_INVALID:
      result = ACE_TEXT_ALWAYS_CHAR ("invalid"); break;
    case MF_TOPOSTATUS_READY:
      result = ACE_TEXT_ALWAYS_CHAR ("ready"); break;
    case MF_TOPOSTATUS_STARTED_SOURCE:
      result = ACE_TEXT_ALWAYS_CHAR ("started"); break;
#if (WINVER >= _WIN32_WINNT_WIN7)
    case MF_TOPOSTATUS_DYNAMIC_CHANGED:
      result = ACE_TEXT_ALWAYS_CHAR ("changed"); break;
#endif
    case MF_TOPOSTATUS_SINK_SWITCHED:
      result = ACE_TEXT_ALWAYS_CHAR ("switched"); break;
    case MF_TOPOSTATUS_ENDED:
      result = ACE_TEXT_ALWAYS_CHAR ("ended"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown topology status (was: %u), aborting\n"),
                  topologyStatus_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
Stream_Module_Device_MediaFoundation_Tools::activateToString (IMFActivate* IMFActivate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::activateToString"));

  std::string result;

  HRESULT result_2 = E_FAIL;
  IMFAttributes* attributes_p = IMFActivate_in;
  WCHAR buffer[BUFSIZ];
  result_2 = attributes_p->GetString (MFT_FRIENDLY_NAME_Attribute,
                                      buffer, sizeof (buffer),
                                      NULL);
  if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to IMFAttributes::GetString(MFT_FRIENDLY_NAME_Attribute): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer));

error:
  return result;
}

//bool
//Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (IMFSourceReaderEx* IMFSourceReaderEx_in,
//                                              const IMFMediaType* IMFMediaType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat"));
//
//  // sanit ycheck(s)
//  ACE_ASSERT (IMFSourceReaderEx_in);
//  ACE_ASSERT (IMFMediaType_in);
//
//  HRESULT result = E_FAIL;
//  struct _GUID GUID_s = { 0 };
//  UINT32 width, height;
//  UINT32 numerator, denominator;
//  result =
//    const_cast<IMFMediaType*> (IMFMediaType_in)->GetGUID (MF_MT_SUBTYPE,
//                                                          &GUID_s);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//  result = MFGetAttributeSize (const_cast<IMFMediaType*> (IMFMediaType_in),
//                               MF_MT_FRAME_SIZE,
//                               &width, &height);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//  result = MFGetAttributeRatio (const_cast<IMFMediaType*> (IMFMediaType_in),
//                                MF_MT_FRAME_RATE,
//                                &numerator, &denominator);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    return false;
//  } // end IF
//
//  DWORD count = 0;
//  IMFMediaType* media_type_p = NULL;
//  struct _GUID GUID_2 = { 0 };
//  UINT32 width_2, height_2;
//  UINT32 numerator_2, denominator_2;
//  DWORD flags = 0;
//  while (result == S_OK)
//  {
//    media_type_p = NULL;
//    result =
//      IMFSourceReaderEx_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                                count,
//                                                &media_type_p);
//    if (result != S_OK) break;
//
//    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    if (GUID_s != GUID_2) goto continue_;
//
//    result = MFGetAttributeSize (media_type_p,
//                                 MF_MT_FRAME_SIZE,
//                                 &width_2, &height_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    if (width != width_2) goto continue_;
//
//    result = MFGetAttributeRatio (media_type_p,
//                                  MF_MT_FRAME_RATE,
//                                  &numerator_2, &denominator_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    if ((numerator   != numerator_2)  ||
//        (denominator != denominator_2)) goto continue_;
//
//    result =
//      IMFSourceReaderEx_in->SetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                                media_type_p,
//                                                &flags);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFSourceReader::SetNativeMediaType(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (media_type_p).c_str ()),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      media_type_p->Release ();
//
//      return false;
//    } // end IF
//    media_type_p->Release ();
//
//    return true;
//
//continue_:
//    media_type_p->Release ();
//
//    ++count;
//  } // end WHILE
//
//  // *NOTE*: this means that the device does not support the requested media
//  //         type 'natively'
//  //         --> try to auto-load a (MFT/DMO) decoder
//  //             see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd389281(v=vs.85).aspx#setting_output_formats
//  if (!Stream_Module_Device_MediaFoundation_Tools::setOutputFormat (IMFSourceReaderEx_in,
//                                                    IMFMediaType_in))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::setOutputFormat(), aborting\n")));
//    goto error;
//  } // end IF
//
//  return true;
//
//error:
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("the source reader does not support the requested media type (was: \"%s\"), aborting\n"),
//              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (IMFMediaType_in).c_str ())));
//
//  // debug info
//  Stream_Module_Device_MediaFoundation_Tools::dump (IMFSourceReaderEx_in);
//
//  return false;
//}
bool
Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (IMFTopology* IMFTopology_in,
                                                              const IMFMediaType* IMFMediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (IMFTopology_in);
  ACE_ASSERT (IMFMediaType_in);

  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));

    // clean up
    collection_p->Release ();

    return false;
  } // end IF
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    unknown_p->Release ();

    return false;
  } // end IF
  unknown_p->Release ();

  IMFMediaSource* media_source_p = NULL;
  result = topology_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&media_source_p));
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release ();
  if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (media_source_p,
                                                     IMFMediaType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n")));

    // clean up
    media_source_p->Release ();

    return false;
  } // end IF
  media_source_p->Release ();

  return true;
}
bool
Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (IMFMediaSource* IMFMediaSource_in,
                                                              const IMFMediaType* IMFMediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (IMFMediaSource_in);
  ACE_ASSERT (IMFMediaType_in);

  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  BOOL is_selected = FALSE;

  HRESULT result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;
  result =
    media_type_handler_p->SetCurrentMediaType (const_cast<IMFMediaType*> (IMFMediaType_in));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::SetCurrentMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_type_handler_p->Release ();
  media_type_handler_p = NULL;

  return true;

error:
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();

  return false;
}

bool
Stream_Module_Device_MediaFoundation_Tools::copyAttribute (const IMFAttributes* source_in,
                                                           IMFAttributes* destination_in,
                                                           REFGUID key_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::copyAttribute"));

  // sanity check(s)
  ACE_ASSERT (source_in);
  ACE_ASSERT (destination_in);

  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);

  HRESULT result =
    const_cast<IMFAttributes*> (source_in)->GetItem (key_in, &property_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::GetItem(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto clean;
  } // end IF
  result = destination_in->SetItem (key_in, property_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::SetItem(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto clean;
  } // end IF

clean:
  PropVariantClear (&property_s);

  return (result == S_OK);
}
bool
Stream_Module_Device_MediaFoundation_Tools::copyMediaType (const IMFMediaType* IMFMediaType_in,
                                                           IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::copyMediaType"));

  // sanity check(s)
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = MFCreateMediaType (&IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  result =
    const_cast<IMFMediaType*> (IMFMediaType_in)->CopyAllItems (IMFMediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::CopyAllItems(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    IMFMediaType_out->Release ();
    IMFMediaType_out = NULL;

    return false;
  } // end IF

  return true;
}

std::string
Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString (REFGUID GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::mediaSubTypeToString"));

  //std::string result;

  //GUID2STRING_MAP_ITERATOR_T iterator =
  //  Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.find (GUID_in);
  //if (iterator == Stream_Module_Device_MediaFoundation_Tools::Stream_MediaSubType2StringMap.end ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
  //              ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (GUID_in).c_str ())));
  //  return result;
  //} // end IF
  //result = (*iterator).second;

  //return result;

  FOURCCMap fourcc_map (&GUID_in);

  return Stream_Module_Decoder_Tools::FOURCCToString (fourcc_map.GetFOURCC ());
}

std::string
Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString"));

  std::string result;

  struct _AMMediaType media_type;
  ACE_OS::memset (&media_type, 0, sizeof (media_type));
  HRESULT result_2 =
    MFInitAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (mediaType_in),
                                      GUID_NULL,
                                      &media_type);
  if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFInitAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return std::string ();
  } // end IF

  result = Stream_Module_Device_DirectShow_Tools::mediaTypeToString (media_type);

  // clean up
  Stream_Module_Device_DirectShow_Tools::freeMediaType (media_type);

  return result;
}
