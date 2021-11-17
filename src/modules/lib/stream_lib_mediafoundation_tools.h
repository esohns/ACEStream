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

#include <list>
#include <string>

#include "d3d9.h"
#include "dxva2api.h"
#include "mfapi.h"
#undef GetObject
#include "mfidl.h"
#include "mfreadwrite.h"
#include "strmif.h"

#include "ace/Global_Macros.h"

#include "common_image_common.h"

#include "stream_lib_common.h"
#include "stream_lib_mediafoundation_common.h"

class Stream_MediaFramework_MediaFoundation_Tools
{
  friend class Stream_MediaFramework_Tools;

 public:
  static void initialize ();

  static struct _GUID toFormat (const IMFMediaType*);
  static Common_Image_Resolution_t toResolution (const IMFMediaType*);
  static unsigned int toFramerate (const IMFMediaType*);

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

  //static bool getOutputFormat (IMFSourceReader*, // source handle
  //                             IMFMediaType*&);  // return value: media type
  // *NOTE*: returns the first RGB or Chroma-Luminance format
  static bool getOutputFormat (IMFTransform*,   // MFT handle
                               IMFMediaType*&); // return value: media type
  // *NOTE*: returns the first available output type of the first output node
  //         (if any), else, starting from the first source node, the first
  //         available output type (of the first stream) of the last connected
  //         node
  static bool getOutputFormat (IMFTopology*,    // topology handle
                               TOPOID,          // node identifier
                               IMFMediaType*&); // return value: media type

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool getTopology (IMFMediaSession*, // media session handle
                           IMFTopology*&);   // return value: topology handle
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
  static bool getMediaSource (const std::string&,  // device identifier
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

  static bool addGrabber (const IMFMediaType*,            // sample grabber sink input media type handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                          IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle
#else
                          IMFSampleGrabberSinkCallback*,  // sample grabber sink callback handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                          IMFTopology*,                   // topology handle
                          TOPOID&);                       // return value: grabber node id
  static bool addRenderer (HWND,         // window handle
                           IMFTopology*, // topology handle
                           TOPOID&);     // return value: renderer node id

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
  static void dump (IMFTransform*); // transform handle

  static bool copy (const IMFAttributes*, // source
                    IMFAttributes*,       // destination
                    REFGUID);             // key
  // *IMPORTANT NOTE*: callers must 'Release' any return values !
  static IMFMediaType* copy (const IMFMediaType*); // media type
  inline static void free (Stream_MediaFramework_MediaFoundation_Formats_t& formats_in) { for (Stream_MediaFramework_MediaFoundation_FormatsIterator_t iterator = formats_in.begin (); iterator != formats_in.end (); ++iterator) (*iterator)->Release (); formats_in.clear (); }
  //static std::string mediaSubTypeToString (REFGUID); // media subtype
  static std::string toString (const IMFMediaType*); // media type
  static std::string toString (MF_TOPOSTATUS); // topology status
  static std::string toString (IMFActivate*); // activate handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  static std::string toString (IMFMediaSourceEx*); // media source handle
#else
  static std::string toString (IMFMediaSource*); // media source handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  //static std::string transformToString (IMFTransform*); // transform handle

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

  typedef std::list<IMFTopologyNode*> TOPOLOGY_PATH_T;
  typedef TOPOLOGY_PATH_T::iterator TOPOLOGY_PATH_ITERATOR_T;
  typedef std::list<TOPOLOGY_PATH_T> TOPOLOGY_PATHS_T;
  typedef TOPOLOGY_PATHS_T::iterator TOPOLOGY_PATHS_ITERATOR_T;
  static bool expand (TOPOLOGY_PATH_T&,   // input/return value: topology path
                      TOPOLOGY_PATHS_T&); // input/return value: topology paths

  static std::string toString (enum MF_TOPOLOGY_TYPE); // node type
};

#endif
