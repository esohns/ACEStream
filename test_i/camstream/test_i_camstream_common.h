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

#include <ace/Synch_Traits.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include <cguid.h>
//#include <evr.h>
//#include <mfapi.h>
//#include <mfidl.h>
////#include "mfobjects.h>
//#include "mfreadwrite.h>
//#include "strmif.h>
#else
#include <linux/videodev2.h>

#include <gtk/gtk.h>
#endif

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "common_ui_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_session_data.h"
#include "stream_statemachine_control.h"

#include "stream_module_net_common.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "net_configuration.h"
#include "net_defines.h"

#include "test_i_common.h"
#include "test_i_gtk_common.h"

#include "test_i_configuration.h"
#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IGraphBuilder;
struct IVideoWindow;
#endif
class Stream_IAllocator;
//template <typename ControlMessageType>
//class Stream_ControlMessage_T;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//class Test_I_Source_DirectShow_Stream_Message;
//class Test_I_Source_DirectShow_Stream_SessionMessage;
//struct Test_I_Source_DirectShow_ConnectionState;
//class Test_I_Source_MediaFoundation_Stream_Message;
//class Test_I_Source_MediaFoundation_Stream_SessionMessage;
//struct Test_I_Source_MediaFoundation_ConnectionState;
//#else
//class Test_I_Source_V4L2_Stream_Message;
//class Test_I_Source_V4L2_Stream_SessionMessage;
//struct Test_I_Source_V4L2_ConnectionState;
//#endif

typedef int Test_I_HeaderType_t;
typedef int Test_I_CommandType_t;

typedef Stream_Statistic Test_I_RuntimeStatistic_t;

typedef Common_IStatistic_T<Test_I_RuntimeStatistic_t> Test_I_StatisticReportingHandler_t;

struct Test_I_CamStream_Configuration;
struct Test_I_StreamConfiguration;
struct Test_I_CamStream_UserData
 : Test_I_UserData
{
  inline Test_I_CamStream_UserData ()
   : Test_I_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_CamStream_Configuration* configuration;
  Test_I_StreamConfiguration*     streamConfiguration;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_CamStream_DirectShow_SessionData
 : Test_I_SessionData
{
  inline Test_I_CamStream_DirectShow_SessionData ()
   : Test_I_SessionData ()
   , direct3DDevice (NULL)
   , format (NULL)
   , resetToken (0)
   , userData (NULL)
  {
    format =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    if (!format)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing\n")));
    else
      ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
  };
  inline Test_I_CamStream_DirectShow_SessionData& operator+= (const Test_I_CamStream_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    // sanity check(s)
    ACE_ASSERT (rhs_in.format);

    if (format)
      Stream_Module_Device_Tools::deleteMediaType (format);
    if (!Stream_Module_Device_Tools::copyMediaType (*rhs_in.format,
                                                    format))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::copyMediaType(), continuing\n")));

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  IDirect3DDevice9Ex*        direct3DDevice;
  struct _AMMediaType*       format;
  UINT                       resetToken; // direct 3D manager 'id'
  Test_I_CamStream_UserData* userData;
};
typedef Stream_SessionData_T<Test_I_CamStream_DirectShow_SessionData> Test_I_CamStream_DirectShow_SessionData_t;
struct Test_I_CamStream_MediaFoundation_SessionData
 : Test_I_SessionData
{
  inline Test_I_CamStream_MediaFoundation_SessionData ()
   : Test_I_SessionData ()
   , direct3DDevice (NULL)
   , format (NULL)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
   //, topology (NULL)
   , userData (NULL)
  {
    format =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    if (!format)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing\n")));
    else
      ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
  };
  inline Test_I_CamStream_MediaFoundation_SessionData& operator+= (const Test_I_CamStream_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    // sanity check(s)
    ACE_ASSERT (rhs_in.format);

    if (format)
      Stream_Module_Device_Tools::deleteMediaType (format);
    if (!Stream_Module_Device_Tools::copyMediaType (*rhs_in.format,
                                                    format))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::copyMediaType(), continuing\n")));

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  IDirect3DDevice9Ex*        direct3DDevice;
  struct _AMMediaType*       format;
  TOPOID                     rendererNodeId;
  UINT                       resetToken; // direct 3D manager 'id'
  IMFMediaSession*           session;
  //IMFTopology*               topology;
  Test_I_CamStream_UserData* userData;
};
typedef Stream_SessionData_T<Test_I_CamStream_MediaFoundation_SessionData> Test_I_CamStream_MediaFoundation_SessionData_t;
struct Test_I_CamStream_SessionData
 : Test_I_SessionData
{
  inline Test_I_CamStream_SessionData ()
   : Test_I_SessionData ()
   , userData (NULL)
  {};
  inline Test_I_CamStream_SessionData& operator+= (const Test_I_CamStream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_I_CamStream_UserData* userData;
};
typedef Stream_SessionData_T<Test_I_CamStream_SessionData> Test_I_CamStream_SessionData_t;
#else
struct Test_I_CamStream_V4L2_SessionData
 : Test_I_SessionData
{
  inline Test_I_CamStream_V4L2_SessionData ()
   : Test_I_SessionData ()
   , format (NULL)
   , frameRate (NULL)
   , userData (NULL)
  {};
  inline Test_I_CamStream_V4L2_SessionData& operator+= (const Test_I_CamStream_V4L2_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    format = rhs_in.format;
    frameRate = rhs_in.frameRate;
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct v4l2_format*        format;
  struct v4l2_fract*         frameRate;
  Test_I_CamStream_UserData* userData;
};
typedef Stream_SessionData_T<Test_I_CamStream_V4L2_SessionData> Test_I_CamStream_V4L2_SessionData_t;
#endif

// forward declarations
struct Test_I_CamStream_Configuration;
struct Test_I_CamStream_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  inline Test_I_CamStream_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , contextID (0)
   , gdkWindow (NULL)
   , lock (NULL)
   , pixelBuffer (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (TEST_I_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
   , window (NULL)
#endif
  {};

  Test_I_CamStream_Configuration* configuration;
  guint                           contextID;
  GdkWindow*                      gdkWindow;
  ACE_SYNCH_MUTEX*                lock;
  GdkPixbuf*                      pixelBuffer;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                            useMediaFoundation;
  HWND                            window;
#endif
};

struct Test_I_CamStream_Configuration
 : Test_I_Configuration
{
  inline Test_I_CamStream_Configuration ()
   : Test_I_Configuration ()
   , moduleHandlerConfiguration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
  {};

  // **************************** stream data **********************************
  Test_I_CamStream_ModuleHandlerConfiguration moduleHandlerConfiguration;
  // *************************** protocol data *********************************
  Net_TransportLayerType                      protocol;
};

struct Test_I_CamStream_GTK_ProgressData
 : Test_I_GTK_ProgressData
{
  inline Test_I_CamStream_GTK_ProgressData ()
   : Test_I_GTK_ProgressData ()
   , transferred (0)
  {};

  size_t transferred; // bytes
};

struct Test_I_CamStream_GTK_CBData
 : Test_I_GTK_CBData
{
  inline Test_I_CamStream_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , configuration (NULL)
   , isFirst (true)
   , pixelBuffer (NULL)
   , progressData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (TEST_I_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
#endif
  {
    progressData.GTKState = this;
  };

  Test_I_CamStream_Configuration*   configuration;
  bool                              isFirst; // first activation ?
  GdkPixbuf*                        pixelBuffer;
  Test_I_CamStream_GTK_ProgressData progressData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                              useMediaFoundation;
#endif
};

struct Test_I_CamStream_ThreadData
 : Test_I_ThreadData
{
  inline Test_I_CamStream_ThreadData ()
   : Test_I_ThreadData ()
   , CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (TEST_I_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
#endif
  {};

  Test_I_CamStream_GTK_CBData* CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                         useMediaFoundation;
#endif
};

#endif
