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

#ifndef TEST_I_SOURCE_COMMON_H
#define TEST_I_SOURCE_COMMON_H

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "evr.h"
#include "mfapi.h"
#include "strmif.h"
#else
#include "linux/videodev2.h"

#include "gtk/gtk.h"
#endif

#include "ace/Time_Value.h"

#include "stream_data_base.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "test_i_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IAMStreamConfig;
struct ISampleGrabber;
#endif
struct Test_I_Source_Configuration;
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_UserData
 : Stream_UserData
{
  inline Test_I_Source_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_Source_Configuration*       configuration;
  Test_I_Source_StreamConfiguration* streamConfiguration;
};

struct Test_I_Source_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Source_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ////////////////////////////////////
   , userData (NULL)
  {};

  Test_I_Source_UserData* userData;
};

struct Test_I_Source_StreamState;
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_Stream_StatisticData;
struct Test_I_Source_Stream_ModuleHandlerConfiguration;
class Test_I_Source_Stream_SessionMessage;
class Test_I_Source_Stream_Message;
typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                      /////////////////
                      ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      /////////////////
                      Stream_StateMachine_ControlState,
                      Test_I_Source_StreamState,
                      /////////////////
                      Test_I_Source_StreamConfiguration,
                      /////////////////
                      Test_I_Source_Stream_StatisticData,
                      /////////////////
                      Stream_ModuleConfiguration,
                      Test_I_Source_Stream_ModuleHandlerConfiguration,
                      /////////////////
                      Test_I_Source_Stream_SessionData,   // session data
                      Test_I_Source_Stream_SessionData_t, // session data container (reference counted)
                      Test_I_Source_Stream_SessionMessage,
                      Test_I_Source_Stream_Message> Test_I_Source_StreamBase_t;
struct Test_I_Source_Stream_ModuleHandlerConfiguration
 : Test_I_Stream_ModuleHandlerConfiguration
{
  inline Test_I_Source_Stream_ModuleHandlerConfiguration ()
   : Test_I_Stream_ModuleHandlerConfiguration ()
   , area ()
   , connection (NULL)
   , connectionManager (NULL)
   , device ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , format (NULL)
#else
   , format ()
   , frameRate ()
#endif
   , socketHandlerConfiguration (NULL)
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , stream (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , windowController (NULL)
#else
   , bufferMap ()
   , buffers (MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS)
   , fileDescriptor (-1)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , v4l2Window (NULL)
#endif
   , window (NULL)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //format =
    //  static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    //if (!format)
    //{
    //  ACE_DEBUG ((LM_CRITICAL,
    //              ACE_TEXT ("failed to allocate memory, continuing\n")));
    //} // end IF
    //else
    //  ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
    HRESULT result = MFCreateMediaType (&format);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#else
    ACE_OS::memset (&format, 0, sizeof (format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ACE_OS::memset (&frameRate, 0, sizeof (frameRate));
#endif
  };

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct tagRECT                            area;
  //IVideoWindow*                             windowController;
  IMFVideoDisplayControl*                   windowController;
#else
  GdkRectangle                              area;
#endif
  Test_I_Source_IConnection_t*              connection; // TCP target/IO module
  Test_I_Source_InetConnectionManager_t*    connectionManager; // TCP IO module
  // *PORTABILITY*: Win32: "FriendlyName" property
  //                UNIX : v4l2 device file (e.g. "/dev/video0" (Linux))
  std::string                               device;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //struct _AMMediaType*                      format;
  IMFMediaType*                             format;
#else
  struct v4l2_format                        format;
  struct v4l2_fract                         frameRate; // time-per-frame (s)
#endif
  Test_I_Source_SocketHandlerConfiguration* socketHandlerConfiguration;
  ACE_Time_Value                            statisticCollectionInterval;
  Test_I_Source_StreamBase_t*               stream;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                                      window;
#else
  INDEX2BUFFER_MAP_T                        bufferMap;
  __u32                                     buffers; // v4l device buffers
  int                                       fileDescriptor;
  v4l2_memory                               method; // v4l camera source
  struct v4l2_window*                       v4l2Window;
  GdkWindow*                                window;
#endif
};

struct Test_I_Source_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_I_Source_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
//   , statisticReportingInterval (0)
   , stream (NULL)
  {};

//  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_StreamBase_t* stream;
};

struct Test_I_Source_Stream_StatisticData
 : Stream_Statistic
{
  inline Test_I_Source_Stream_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif
  {};

  inline Test_I_Source_Stream_StatisticData operator+= (const Test_I_Source_Stream_StatisticData& rhs_in)
  {
    Stream_Statistic::operator+= (rhs_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    capturedFrames += rhs_in.capturedFrames;
#endif

    return *this;
  };

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  unsigned int capturedFrames;
#endif
};

struct Test_I_Source_Stream_SessionData
 : Test_I_Stream_SessionData
{
  inline Test_I_Source_Stream_SessionData ()
   : Test_I_Stream_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};

  inline Test_I_Source_Stream_SessionData& operator+= (const Test_I_Source_Stream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_Stream_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState 
                                       : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_I_Source_ConnectionState* connectionState;
  Test_I_Source_UserData*        userData;
};
typedef Stream_SessionData_T<Test_I_Source_Stream_SessionData> Test_I_Source_Stream_SessionData_t;

struct Test_I_Source_StreamConfiguration
 : Stream_Configuration
{
  inline Test_I_Source_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_I_Source_Stream_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Test_I_Source_StreamState
 : Stream_State
{
  inline Test_I_Source_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_I_Source_Stream_SessionData* currentSessionData;
  Test_I_Source_UserData*           userData;
};

struct Test_I_Source_Configuration
 : Test_I_Configuration
{
  inline Test_I_Source_Configuration ()
   : Test_I_Configuration ()
   , signalHandlerConfiguration ()
   , socketHandlerConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** signal data **********************************
  Test_I_Source_SignalHandlerConfiguration        signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_Source_SocketHandlerConfiguration        socketHandlerConfiguration;
  // **************************** stream data **********************************
  Test_I_Source_Stream_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_I_Source_StreamConfiguration               streamConfiguration;

  Test_I_Source_UserData                          userData;
};

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,

                                          Test_I_Source_Stream_Message,
                                          Test_I_Source_Stream_SessionMessage> Test_I_Source_MessageAllocator_t;

typedef Common_INotify_T<unsigned int,
                         Test_I_Source_Stream_SessionData,
                         Test_I_Source_Stream_Message,
                         Test_I_Source_Stream_SessionMessage> Test_I_Source_IStreamNotify_t;
typedef std::list<Test_I_Source_IStreamNotify_t*> Test_I_Source_Subscribers_t;
typedef Test_I_Source_Subscribers_t::iterator Test_I_Source_SubscribersIterator_t;

typedef Common_ISubscribe_T<Test_I_Source_IStreamNotify_t> Test_I_Source_ISubscribe_t;

struct Test_I_Source_GTK_CBData
 : Test_I_GTK_CBData
{
  inline Test_I_Source_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , configuration (NULL)
   , isFirst (true)
   , stream (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , streamConfiguration (NULL)
#else
   , device (-1)
#endif
   , UDPStream (NULL)
  {};

  Test_I_Source_Configuration* configuration;
  bool                         isFirst; // first activation ?
  Test_I_Source_StreamBase_t*  stream;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IAMStreamConfig*             streamConfiguration;
#else
  int                          device; // (capture) device file descriptor
#endif
  Test_I_Source_Subscribers_t  subscribers;
  Test_I_Source_StreamBase_t*  UDPStream;
};

struct Test_I_Source_ThreadData
{
  inline Test_I_Source_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
  {};

  Test_I_Source_GTK_CBData* CBData;
  guint                     eventSourceID;
};

#endif
