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

#ifndef STREAM_MODULE_DEV_MEDIAFOUNDATION_TOOLS_H
#define STREAM_MODULE_DEV_MEDIAFOUNDATION_TOOLS_H

#include <map>
#include <string>

#include "ace/Global_Macros.h"

#include <cguid.h>
#include <d3d9.h>
#include <dxva2api.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include <strmif.h>
#include <mtype.h>

#include "stream_dec_common.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"
#include "stream_dev_exports.h"

class Stream_Dev_Export Stream_Module_Device_MediaFoundation_Tools
{
 public:
  static void initialize ();

  // *NOTE*: 'tees' the upstream node of the first output node (if any); else,
  //         starting from the first source node, 'tee' the last connected node
  //         from the first stream
  static bool append (IMFTopology*, // topology handle
                      TOPOID);      // topology node id
  static bool clear (IMFMediaSession*, // media session handle
                     bool = true);     // wait for completion ?
  // *NOTE*: removes all 'transform' type MFTs (and any connected downstream
  //         nodes)
  static bool clear (IMFTopology*); // topology handle
  // *NOTE*: disconnects all downstream nodes as well
  static bool disconnect (IMFTopologyNode*); // topology node handle

  // -------------------------------------

  //static bool getCaptureFormat (IMFSourceReader*, // source handle
  //                              IMFMediaType*&);  // return value: media type
  //static bool setCaptureFormat (IMFSourceReaderEx*,   // source handle
  //                              const IMFMediaType*); // media type
  static bool getCaptureFormat (IMFMediaSource*, // source handle
                                IMFMediaType*&);  // return value: media type
  static bool setCaptureFormat (IMFTopology*,         // topology handle
                                const IMFMediaType*); // media type
  //static bool getOutputFormat (IMFSourceReader*, // source handle
  //                             IMFMediaType*&);  // return value: media type
  // *NOTE*: 
  static bool getOutputFormat (IMFTransform*,   // MFT handle
                               IMFMediaType*&); // return value: media type
  // *NOTE*: returns the first available output type of the first output node
  //         (if any), else, starting from the first source node, the first
  //         available output type (of the first stream) of the last connected
  //         node
  static bool getOutputFormat (IMFTopology*,    // topology handle
                               TOPOID,          // node identifier
                               IMFMediaType*&); // return value: media type

  static bool getMediaSource (const std::string&, // device name ("FriendlyName")
                              REFGUID,            // device category
                              IMFMediaSource*&,   // return value: media device handle
                              WCHAR*&,            // return value: symbolic link
                              UINT32&);           // return value: symbolic link size
  static bool getMediaSource (const IMFMediaSession*, // media session handle
                              IMFMediaSource*&);      // return value: media source handle
  static bool getMediaSource (const IMFTopology*, // topology handle
                              IMFMediaSource*&);  // return value: media source handle
  //// *TODO*: using the Direct3D device manager (used by the EVR renderer) is
  ////         currently broken
  ////         --> pass NULL and use a different visualization module (e.g. the
  ////             Direct3D module)
  //static bool getSourceReader (IMFMediaSource*&,               // media device handle (in/out)
  //                             WCHAR*&,                        // return value: symbolic link
  //                             UINT32&,                        // return value: symbolic link size
  //                             const IDirect3DDeviceManager9*, // Direct3D device manager handle
  //                             const IMFSourceReaderCallback*, // callback handle
  //                             bool,                           // capture media type is YUV ?
  //                             IMFSourceReaderEx*&);           // return value: source reader handle
  static bool getSampleGrabberNodeId (const IMFTopology*, // topology handle
                                      TOPOID&);           // return value: topology node id

  static bool loadDeviceTopology (const std::string&,                   // device name ("FriendlyName")
                                  REFGUID,                              // device category
                                  IMFMediaSource*&,                     // input/return value: (capture) media source handle
                                  const IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
                                  IMFTopology*&);                       // return value: topology handle
  static bool loadSourceTopology (const std::string&, // URL
                                  IMFMediaSource*&,   // input/return value: media source handle
                                  IMFTopology*&);     // return value: topology handle
  static bool loadSourceTopology (IMFMediaSource*, // media source handle
                                  IMFTopology*&);  // return value: topology handle

  // -------------------------------------

  static bool addGrabber (const IMFMediaType*,                  // sample grabber sink input media type handle
                          const IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle
                          IMFTopology*,                         // topology handle
                          TOPOID&);                             // return value: grabber node id
  static bool addRenderer (const HWND,   // window handle
                           IMFTopology*, // topology handle
                           TOPOID&);     // return value: renderer node id
  static bool loadAudioRendererTopology (const std::string&,                   // device name ("FriendlyName")
                                         IMFMediaType*,                        // [return value] sample grabber sink input media type handle
                                         const IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
                                         int,                                  // audio output handle [0: do not use tee/renderer]
                                         TOPOID&,                              // return value: sample grabber sink node id
                                         TOPOID&,                              // return value: audio renderer sink node id
                                         IMFTopology*&);                       // input/return value: topology handle
  static bool loadVideoRendererTopology (const std::string&,                   // device name ("FriendlyName")
                                         const IMFMediaType*,                  // sample grabber sink input media type handle
                                         const IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
                                         const HWND,                           // window handle [NULL: do not use tee/EVR]
                                         TOPOID&,                              // return value: sample grabber sink node id
                                         TOPOID&,                              // return value: EVR sink node id
                                         IMFTopology*&);                       // input/return value: topology handle
  static bool loadVideoRendererTopology (const IMFMediaType*, // input media type handle
                                         const HWND,          // window handle [NULL: do not use tee/EVR]
                                         TOPOID&,             // return value: EVR sink node id
                                         IMFTopology*&);      // input/return value: topology handle

  static bool loadTargetRendererTopology (const std::string&,  // URL
                                          const IMFMediaType*, // media source output media type handle
                                          const HWND,          // window handle [NULL: do not use tee/EVR]
                                          TOPOID&,             // return value: EVR sink node id
                                          IMFTopology*&);      // input/return value: topology handle
  static bool setTopology (IMFTopology*,      // topology handle
                           IMFMediaSession*&, // input/return value: media session handle
                           bool = false,      // resolve topology ? (uses IMFTopoLoader)
                           bool = true);      // wait for completion ?

  // -------------------------------------

  //static void dump (IMFSourceReader*); // source reader handle
  static void dump (IMFTopology*); // topology handle
  static void dump (IMFTransform*); // transform handle

  static bool copyAttribute (const IMFAttributes*, // source
                             IMFAttributes*,       // destination
                             REFGUID);             // key
  static bool copyMediaType (const IMFMediaType*, // media type
                             IMFMediaType*&);     // return value: handle
  //static std::string mediaSubTypeToString (REFGUID); // media subtype
  static std::string mediaTypeToString (const IMFMediaType*); // media type
  static std::string topologyStatusToString (MF_TOPOSTATUS); // topology status
  static std::string activateToString (IMFActivate*); // activate handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_MediaFoundation_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_MediaFoundation_Tools (const Stream_Module_Device_MediaFoundation_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_MediaFoundation_Tools& operator= (const Stream_Module_Device_MediaFoundation_Tools&))

  struct less_guid
  {
    bool operator () (const struct _GUID& lhs_in,
                      const struct _GUID& rhs_in) const
    {
      //ACE_ASSERT (lhs_in.Data2 == rhs_in.Data2);
      //ACE_ASSERT (lhs_in.Data3 == rhs_in.Data3);
      //ACE_ASSERT (*(long long*)lhs_in.Data4 == *(long long*)rhs_in.Data4);

      return (lhs_in.Data1 < rhs_in.Data1);
    }
  };
  typedef std::map<struct _GUID, std::string, less_guid> GUID2STRING_MAP_T;
  typedef GUID2STRING_MAP_T::const_iterator GUID2STRING_MAP_ITERATOR_T;
  static GUID2STRING_MAP_T Stream_MediaMajorType2StringMap;
  static GUID2STRING_MAP_T Stream_MediaSubType2StringMap;

  static bool enableDirectXAcceleration (IMFTopology*); // topology handle
  static bool setCaptureFormat (IMFMediaSource*,      // source handle
                                const IMFMediaType*); // media type
  //// *NOTE*: (if the media type is not a 'native' format) "... The Source Reader
  ////         will automatically load the decoder. ..."
  //static bool setOutputFormat (IMFSourceReader*,     // source handle
  //                             const IMFMediaType*); // media type

  typedef std::list<IMFTopologyNode*> TOPOLOGY_PATH_T;
  typedef TOPOLOGY_PATH_T::iterator TOPOLOGY_PATH_ITERATOR_T;
  typedef std::list<TOPOLOGY_PATH_T> TOPOLOGY_PATHS_T;
  typedef TOPOLOGY_PATHS_T::iterator TOPOLOGY_PATHS_ITERATOR_T;
  static bool expand (TOPOLOGY_PATH_T&,   // input/return value: topology path
                      TOPOLOGY_PATHS_T&); // input/return value: topology paths

  static std::string nodeTypeToString (enum MF_TOPOLOGY_TYPE); // node type
};

#endif
