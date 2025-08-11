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

#ifndef TEST_I_CAMSTREAM_COMMON_H
#define TEST_I_CAMSTREAM_COMMON_H

#include <limits>
#include <map>
#include <set>
#include <string>

#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "d3d9.h"
#undef GetObject
#include "mfidl.h"
#include "strmif.h"
#else
#include "linux/videodev2.h"

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "common.h"
#include "common_inotify.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"

#include "common_parser_common.h"

#include "common_time_common.h"

#include "common_ui_common.h"
#include "common_ui_windowtype_converter.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_session_data.h"
#include "stream_statemachine_control.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_common.h"
#include "stream_lib_directshow_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_net_common.h"

//#include "net_configuration.h"
#include "net_defines.h"

#include "test_i_common.h"
#include "test_i_configuration.h"
//#include "test_i_connection_common.h"
//#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT

#include "test_i_camstream_defines.h"

// forward declarations
class Stream_IAllocator;

typedef int Test_I_HeaderType_t;
typedef int Test_I_CommandType_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_CamStream_DirectShow_SessionData
 : Test_I_DirectShow_SessionData
{
  Test_I_CamStream_DirectShow_SessionData ()
   : Test_I_DirectShow_SessionData ()
   , connectionStates ()
   , direct3DDevice (NULL)
   , resetToken (0)
  {}

  // *NOTE*: called on stream link after connecting; 'this' is upstream
  struct Test_I_CamStream_DirectShow_SessionData& operator+= (const struct Test_I_CamStream_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_DirectShow_SessionData::operator+= (rhs_in);

    connectionStates.insert (rhs_in.connectionStates.begin (),
                             rhs_in.connectionStates.end ());

    return *this;
  }

  Stream_Net_ConnectionStates_t     connectionStates;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*               direct3DDevice;
#else
  IDirect3DDevice9*                 direct3DDevice;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  UINT                              resetToken; // direct 3D manager 'id'
};
//typedef Stream_SessionData_T<Test_I_CamStream_DirectShow_SessionData> Test_I_CamStream_DirectShow_SessionData_t;

struct Test_I_CamStream_MediaFoundation_SessionData
 : Test_I_MediaFoundation_SessionData
{
  Test_I_CamStream_MediaFoundation_SessionData ()
   : Test_I_MediaFoundation_SessionData ()
   , connectionStates ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , rendererNodeId (0)
   , session (NULL)
   //, topology (NULL)
  {}

  struct Test_I_CamStream_MediaFoundation_SessionData& operator+= (const struct Test_I_CamStream_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_MediaFoundation_SessionData::operator+= (rhs_in);

    connectionStates.insert (rhs_in.connectionStates.begin (),
                             rhs_in.connectionStates.end ());

    return *this;
  }

  Stream_Net_ConnectionStates_t     connectionStates;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*               direct3DDevice;
#else
  IDirect3DDevice9*                 direct3DDevice;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  UINT                              direct3DManagerResetToken;
  TOPOID                            rendererNodeId;
  IMFMediaSession*                  session;
  //IMFTopology*               topology;
};
//typedef Stream_SessionData_T<Test_I_CamStream_MediaFoundation_SessionData> Test_I_CamStream_MediaFoundation_SessionData_t;
#else
struct Test_I_CamStream_V4L_SessionData
 : Test_I_SessionData
{
  Test_I_CamStream_V4L_SessionData ()
   : Test_I_SessionData ()
  {}

  struct Test_I_CamStream_V4L_SessionData& operator+= (const struct Test_I_CamStream_V4L_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    return *this;
  }
};
//typedef Stream_SessionData_T<Test_I_CamStream_V4L_SessionData> Test_I_CamStream_V4L_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
struct Test_I_CamStream_Configuration;
struct Test_I_CamStream_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_CamStream_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , configuration (NULL)
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
   , deviceIdentifier ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DConfiguration (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , display ()
   , fullScreen (false)
   , window ()
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    deviceIdentifier.identifier._guid = GUID_NULL;
    deviceIdentifier.identifierDiscriminator = Stream_Device_Identifier::GUID;
#else
    deviceIdentifier.identifier =
      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  }

  struct Test_I_CamStream_Configuration*               configuration;
#if defined (GTK_USE)
  guint                                                contextId;
#endif // GTK_USE
  struct Stream_Device_Identifier                      deviceIdentifier;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_Direct3D_Configuration* direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_UI_DisplayDevice                       display;
  bool                                                 fullScreen;
  union Common_UI_Window                               window;
};

struct Test_I_CamStream_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_CamStream_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , allocatorConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DConfiguration ()
#endif // ACE_WIN32 || ACE_WIN64
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
  {}

  struct Common_AllocatorConfiguration                allocatorConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  // *************************** protocol data *********************************
  enum Net_TransportLayerType                         protocol;
};

//////////////////////////////////////////

struct Test_I_CamStream_UI_ProgressData
 : Test_I_UI_ProgressData
{
  Test_I_CamStream_UI_ProgressData ()
   : Test_I_UI_ProgressData ()
   , transferred (0)
  {}

  size_t transferred; // bytes
};

struct Test_I_CamStream_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_CamStream_UI_CBData ()
   : Test_I_UI_CBData ()
   , configuration (NULL)
   , isFirst (true)
#if defined (GTK_USE)
   , dispatch (NULL)
#endif // GTK_USE
   , progressData ()
  {}

  struct Test_I_CamStream_Configuration*  configuration;
  bool                                    isFirst; // first activation ?
#if defined (GTK_USE)
  Common_IDispatch*                       dispatch;
#endif // GTK_USE
  struct Test_I_CamStream_UI_ProgressData progressData;
};

struct Test_I_CamStream_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_CamStream_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_CamStream_UI_CBData* CBData;
};

#endif
