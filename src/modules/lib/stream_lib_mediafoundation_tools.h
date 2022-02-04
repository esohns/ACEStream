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

#ifndef STREAM_LIB_MEDIAFOUNDATION_TOOLS_H
#define STREAM_LIB_MEDIAFOUNDATION_TOOLS_H

#include <string>

#include "guiddef.h"
#include "mfapi.h"
#undef GetObject
#include "mfidl.h"
#include "mfobjects.h"
#include "mftransform.h"
#include "mmreg.h"

#include "ace/Global_Macros.h"

#include "common_image_common.h"

#include "stream_lib_common.h"
#include "stream_lib_mediafoundation_common.h"

class Stream_MediaFramework_MediaFoundation_Tools
{
  friend class Stream_MediaFramework_Tools;

 public:
  static void initialize ();

  // device identifier
  // *TODO*: move to dev/
  static std::string identifierToString (REFGUID,  // device identifier
                                         REFGUID); // device category

  // format
  static bool canRender (const IMFMediaType*, // media type handle
                         IMFMediaType*&);     // return value: closest match
  static bool has (const IMFMediaType*, // media type handle
                   REFGUID);            // attribute identifier
  static bool isPartial (const IMFMediaType*); // media type handle
  // *NOTE*: test whether either argument is a 'part' of the other
  //         i.e. has <= #attributes, AND the attribute values match
  static bool isPartOf (const IMFMediaType*,  // media type handle 1
                        const IMFMediaType*); // media type handle 2
  // *TODO*: this (probably) does the same as 'isPartOf' (see above)
  static bool match (const IMFMediaType*,  // media type handle 1
                     const IMFMediaType*); // media type handle 2
  // *IMPORTANT NOTE*: only the attributes that do NOT exist in the target are
  //                   merged
  static bool merge (const IMFMediaType*, // source
                     IMFMediaType*,       // target
                     bool = true);        // reconfigure ?
  static bool reconfigure (IMFMediaType*); // media type handle
  // *IMPORTANT NOTE*: make sure to Release() the return value
  static IMFMediaType* to (const struct tWAVEFORMATEX&); // media type
  static struct _GUID toFormat (const IMFMediaType*);
  static Common_Image_Resolution_t toResolution (const IMFMediaType*);
  static unsigned int toFramerate (const IMFMediaType*);

  // topology node
  // *NOTE*: disconnects all downstream nodes as well
  static bool disconnect (IMFTopologyNode*); // topology node handle
  static void shutdown (IMFTopologyNode*); // topology node handle

  // topology
  static void clean (TOPOLOGY_PATH_T&); // list of nodes
  static void clean (TOPOLOGY_PATHS_T&); // list of branches
  static bool hasTee (TOPOLOGY_PATH_T&,   // topology path
                      IMFTopologyNode*&); // return value: tee node handle
  static bool parse (const IMFTopology*, // topology handle
                     TOPOLOGY_PATHS_T&); // return value: list of branches

  // *NOTE*: if there are no source nodes, abort
  //         if the xth branch does not have a sink, append to its last
  //         connected node
  //         else (i.e. all branches have sinks) if the xth branch has a tee
  //         node, add an output and append there
  //         else (i.e. all branches have sinks and there are no tees) tee the
  //         upstream node of the first sink and append there
  static bool append (IMFTopology*, // topology handle
                      TOPOID,       // topology node id
                      bool = true); // set (input) format ?
  static bool clear (IMFTopology*, // topology handle
                     bool = true); // shutdown nodes ?
  // *NOTE*: removes all 'transform' type MFTs (and any connected downstream
  //         nodes)
  static bool clearTransforms (IMFTopology*); // topology handle
  // *NOTE*: (disconnects the topology and-) removes all (but a specific source-) nodes
  static bool reset (IMFTopology*, // topology handle
                     REFGUID);     // retain (device) category source (GUID_NULL: retain first filter w/o input pins)

  // session
  static bool clear (IMFMediaSession*, // media session handle
                     bool = true);     // wait for completion ?
  static void shutdown (IMFMediaSession*); // media session handle

  // -------------------------------------

  static bool getInputFormat (IMFTopology*,    // topology handle
                              TOPOID,          // node identifier
                              IMFMediaType*&); // return value: media type
  static bool setInputFormat (IMFTopology*,   // topology handle
                              TOPOID,         // node identifier
                              IMFMediaType*); // media type
  //static bool getOutputFormat (IMFSourceReader*, // source handle
  //                             IMFMediaType*&);  // return value: media type
  // *NOTE*: video: returns the current-, first RGB/Chroma-Luminance/available
  //         audio: returns the current-, first PCM/IEEE Float/available
  //         format of the first output stream
  static bool getOutputFormat (IMFTransform*,  // MFT handle
                               IMFMediaType*&, // return value: media type
                               bool&);         // return value: current format ? : available-
  // *NOTE*: iff the node id is 0: returns the first available output type of
  //         the first output node (if any), else, starting from the first
  //         source node, the first available output type (of the first stream)
  //         of the last connected node
  static bool getOutputFormat (IMFTopology*,    // topology handle
                               TOPOID,          // node identifier
                               IMFMediaType*&); // return value: media type
  static bool setOutputFormat (IMFTopology*,         // topology handle
                               TOPOID,               // node identifier
                               const IMFMediaType*); // media type

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool getTopology (IMFMediaSession*, // media session handle
                           IMFTopology*&);   // return value: (current- !) topology handle
  static bool getMediaSource (IMFMediaSession*,    // media session handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                              IMFMediaSourceEx*&); // return value: media source handle
#else
                              IMFMediaSource*&);   // return value: media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  static bool getMediaSource (const IMFTopology*,  // topology handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                              IMFMediaSourceEx*&); // return value: media source handle
#else
                              IMFMediaSource*&);      // return value: media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  static bool getMediaSource (REFGUID,             // device identifier
                              REFGUID,             // device category
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                              IMFMediaSourceEx*&); // return value: media source handle
#else
                              IMFMediaSource*&);   // return value: media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool loadSourceTopology (const std::string&, // URL
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                  IMFMediaSourceEx*&, // input/return value: media source handle
#else
                                  IMFMediaSource*&,   // input/return value: media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                  IMFTopology*&);     // return value: topology handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  static bool loadSourceTopology (IMFMediaSourceEx*, // media source handle
#else
  static bool loadSourceTopology (IMFMediaSource*,   // media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                  IMFTopology*&);    // return value: topology handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  // -------------------------------------

  static bool addResampler (const IMFMediaType*, // resampler sink output media type handle
                            IMFTopology*,        // topology handle
                            TOPOID&);            // return value: renderer node id
  static bool addGrabber (
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                          IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle
#else
                          IMFSampleGrabberSinkCallback*,  // sample grabber sink callback handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                          IMFTopology*,                   // topology handle
                          TOPOID&);                       // return value: grabber node id
  static bool addRenderer (REFGUID,      // (major) media type (audio/video)
                           HWND,         // window handle (video only)
                           REFGUID,      // device identifier (audio only)
                           IMFTopology*, // topology handle
                           TOPOID&,      // return value: renderer node id
                           bool = true); // set (input) format ?

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool setTopology (IMFTopology*,      // topology handle
                           IMFMediaSession*&, // input/return value: media session handle
                           bool = false,      // resolve topology ? (uses IMFTopoLoader)
                           bool = true);      // wait for completion ?
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  static bool enableDirectXAcceleration (IMFTopology*); // topology handle

  // -------------------------------------

  //static void dump (IMFSourceReader*); // source reader handle
  static void dump (IMFTopology*); // topology handle
  static void dump (IMFAttributes*); // attributes handle
  static void dump (IMFTransform*); // transform handle
  static void dump (IMFStreamSink*); // stream sink handle

  static bool copy (const IMFAttributes*, // source
                    IMFAttributes*,       // destination
                    REFGUID);             // key
  // *IMPORTANT NOTE*: callers must 'Release' any return values !
  static IMFMediaType* copy (const IMFMediaType*); // media type
  inline static void free (Stream_MediaFramework_MediaFoundation_Formats_t& formats_in) { for (Stream_MediaFramework_MediaFoundation_FormatsIterator_t iterator = formats_in.begin (); iterator != formats_in.end (); ++iterator) (*iterator)->Release (); formats_in.clear (); }
  //static std::string mediaSubTypeToString (REFGUID); // media subtype
  static std::string toString (const IMFMediaType*, // media type
                               bool = false);       // condensed ?
  static std::string toString (MF_TOPOSTATUS); // topology status
  static std::string toString (IMFActivate*); // activate handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  static std::string toString (IMFMediaSourceEx*); // media source handle
#else
  static std::string toString (IMFMediaSource*); // media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  static std::string toString (IMFTransform*); // transform handle

  // *NOTE*: this wraps MFTEnum()/MFTEnumEx()
  static bool load (REFGUID,                       // category
                    UINT32,                        // flags
                    const MFT_REGISTER_TYPE_INFO*, // input media type {NULL: all}
                    const MFT_REGISTER_TYPE_INFO*, // output media type {NULL: all}
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                    IMFActivate**&,                // return value: module handles
#else
                    IMFAttributes*,                // attributes
                    CLSID*&,                       // return value: module handles
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                    UINT32&);                      // return value: number of handles

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Tools (const Stream_MediaFramework_MediaFoundation_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Tools& operator= (const Stream_MediaFramework_MediaFoundation_Tools&))

  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaMajorTypeToStringMap;
  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaSubTypeToStringMap;

  //// *NOTE*: (if the media type is not a 'native' format) "... The Source Reader
  ////         will automatically load the decoder. ..."
  //static bool setOutputFormat (IMFSourceReader*,     // source handle
  //                             const IMFMediaType*); // media type

  static void expand (const TOPOLOGY_PATH_T&,    // input/return value: topology path
                      TOPOLOGY_PATH_ITERATOR_T&, // iterator
                      TOPOLOGY_PATHS_T&);        // input/return value: topology paths

  static std::string toString (MediaEventType); // event type
  static std::string toString (enum MF_TOPOLOGY_TYPE); // node type
  static std::string toString_2 (const IMFMediaType*); // media type
};

#endif
