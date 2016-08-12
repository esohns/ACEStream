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

#include <deque>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>

#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "cguid.h"
#include "evr.h"
#include "mfapi.h"
#include "mfidl.h"
//#include "mfobjects.h"
#include "mfreadwrite.h"
#include "strmif.h"
#else
#include "linux/videodev2.h"

#include "gtk/gtk.h"
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

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
//#include "test_i_message.h"
//#include "test_i_session_message.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IGraphBuilder;
struct IVideoWindow;
#endif
class Stream_IAllocator;
//template <typename ControlMessageType>
//class Stream_ControlMessage_T;
class Test_I_Source_Stream_Message;
class Test_I_Source_Stream_SessionMessage;
struct Test_I_ConnectionState;

typedef int Test_I_HeaderType_t;
typedef int Test_I_CommandType_t;

typedef Stream_Statistic Test_I_RuntimeStatistic_t;

typedef Common_IStatistic_T<Test_I_RuntimeStatistic_t> Test_I_StatisticReportingHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMediaSample;
#endif
struct Test_I_Source_MessageData
{
  inline Test_I_Source_MessageData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   : sample (NULL)
   , sampleTime (0)
#else
   : device (-1)
   , index (0)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
#endif
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //IMediaSample* sample;
  //double        sampleTime;
  IMFSample* sample;
  LONGLONG   sampleTime;
#else
  int           device; // (capture) device file descriptor
  __u32         index;  // 'index' field of v4l2_buffer
  v4l2_memory   method;
  bool          release;
#endif
};
typedef Stream_DataBase_T<Test_I_Source_MessageData> Test_I_Source_MessageData_t;

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

struct Test_I_CamStream_SessionData
 : Test_I_SessionData
{
  inline Test_I_CamStream_SessionData ()
   : Test_I_SessionData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DDevice (NULL)
   , format (NULL)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
   , topology (NULL)
#else
   , format ()
   , frameRate ()
#endif
   , userData (NULL)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //format =
    //  static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    //if (!format)
    //  ACE_DEBUG ((LM_CRITICAL,
    //              ACE_TEXT ("failed to allocate memory, continuing\n")));
    //else
    //  ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
    HRESULT result = MFCreateMediaType (&format);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#endif
  };
  inline Test_I_CamStream_SessionData& operator+= (const Test_I_CamStream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // sanity check(s)
//    ACE_ASSERT (rhs_in.format);

//    if (format)
//    {
//      format->Release ();
//      format = NULL;
//    } // end IF

//    //if (!Stream_Module_Device_Tools::copyMediaType (*rhs_in.format,
//    //                                                format))
//    //  ACE_DEBUG ((LM_ERROR,
//    //              ACE_TEXT ("failed to Stream_Module_Device_Tools::copyMediaType(), continuing\n")));
//    struct _AMMediaType media_type;
//    ACE_OS::memset (&media_type, 0, sizeof (media_type));
//    HRESULT result = MFInitAMMediaTypeFromMFMediaType (rhs_in.format,
//                                                       GUID_NULL,
//                                                       &media_type);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFInitAMMediaTypeFromMFMediaType(): \"%s\", continuing\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//      goto continue_;
//    } // end IF

//    result = MFInitMediaTypeFromAMMediaType (format,
//                                             &media_type);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFInitMediaTypeFromAMMediaType(): \"%s\", continuing\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

//      // clean up
//      Stream_Module_Device_Tools::freeMediaType (media_type);

//      goto continue_;
//    } // end IF

//    // clean up
//    Stream_Module_Device_Tools::freeMediaType (media_type);
//  continue_:
#else
//    format = rhs_in.format;
#endif
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct _AMMediaType*   format;
  IDirect3DDevice9Ex*        direct3DDevice;
  IMFMediaType*              format;
  TOPOID                     rendererNodeId;
  UINT                       resetToken; // direct 3D manager 'id'
  IMFMediaSession*           session;
  IMFTopology*               topology;
#else
  struct v4l2_format         format;
  struct v4l2_fract          frameRate;
#endif
  Test_I_CamStream_UserData* userData;
};
typedef Stream_SessionData_T<Test_I_CamStream_SessionData> Test_I_CamStream_SessionData_t;

// forward declarations
struct Test_I_CamStream_Configuration;
struct Test_I_CamStream_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  inline Test_I_CamStream_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   //, builder (NULL)
   , mediaSource (NULL)
   , sampleGrabberNodeId (0)
   //, sourceReader (NULL)
   , session (NULL)
//   , topology (NULL)
#endif
   , configuration (NULL)
   //, connection (NULL)
   //, connectionManager (NULL)
   , contextID (0)
   , gdkWindow (NULL)
   , lock (NULL)
   , pixelBuffer (NULL)
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //IGraphBuilder*                            builder;
  IMFMediaSource*                           mediaSource;
  TOPOID                                    sampleGrabberNodeId;
  //IMFSourceReaderEx*                        sourceReader;
  IMFMediaSession*                          session;
//  IMFTopology*                              topology;
#else
#endif
  Test_I_CamStream_Configuration*           configuration;
  //Test_I_IConnection_t*                     connection; // TCP target/IO module
  //Test_I_Stream_InetConnectionManager_t*    connectionManager; // TCP IO module
  guint                                     contextID;
  GdkWindow*                                gdkWindow;
  ACE_SYNCH_MUTEX*                          lock;
  GdkPixbuf*                                pixelBuffer;
};

struct Test_I_CamStream_Configuration
 : Test_I_Configuration
{
  inline Test_I_CamStream_Configuration ()
   : Test_I_Configuration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
  {};

  // *************************** protocol data *********************************
  Net_TransportLayerType                   protocol;
};

enum Test_I_GTK_Event
{
  TEST_I_GKTEVENT_INVALID = -1,
  // ------------------------------------
  TEST_I_GTKEVENT_START = 0,
  TEST_I_GTKEVENT_DATA,
  TEST_I_GTKEVENT_END,
  TEST_I_GTKEVENT_STATISTIC,
  // ------------------------------------
  TEST_I_GTKEVENT_MAX
};
typedef std::deque<Test_I_GTK_Event> Test_I_GTK_Events_t;
typedef Test_I_GTK_Events_t::const_iterator Test_I_GTK_EventsIterator_t;

typedef std::map<guint, ACE_Thread_ID> Test_I_PendingActions_t;
typedef Test_I_PendingActions_t::iterator Test_I_PendingActionsIterator_t;
typedef std::set<guint> Test_I_CompletedActions_t;
typedef Test_I_CompletedActions_t::iterator Test_I_CompletedActionsIterator_t;
struct Test_I_GTK_ProgressData
{
  inline Test_I_GTK_ProgressData ()
    : completedActions ()
    //   , cursorType (GDK_LAST_CURSOR)
    , GTKState (NULL)
    , pendingActions ()
    , statistic ()
    , transferred (0)
    , size (0)
  {};

  Test_I_CompletedActions_t completedActions;
  //  GdkCursorType                      cursorType;
  Common_UI_GTKState*       GTKState;
  Test_I_PendingActions_t   pendingActions;
  Stream_Statistic          statistic;
  size_t                    transferred; // bytes
  size_t                    size; // bytes
};

struct Test_I_GTK_CBData
  : Common_UI_GTKState
{
  inline Test_I_GTK_CBData ()
    : Common_UI_GTKState ()
    , configuration (NULL)
    , eventStack ()
    , progressData ()
    , progressEventSourceID (0)
  {};

  Test_I_CamStream_Configuration* configuration;
  Test_I_GTK_Events_t             eventStack;
  Test_I_GTK_ProgressData         progressData;
  guint                           progressEventSourceID;
};

struct Test_I_CamStream_GTK_CBData
 : Test_I_GTK_CBData
{
  inline Test_I_CamStream_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , pixelBuffer (NULL)
  {};

  GdkPixbuf* pixelBuffer;
};

#endif
