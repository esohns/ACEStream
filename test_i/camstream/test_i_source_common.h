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

#include <ace/config-lite.h>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <evr.h>
#include <mfapi.h>
#include <strmif.h>
#else
#include <linux/videodev2.h>
#endif

#include <gtk/gtk.h>

#include <ace/Time_Value.h>

#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_messageallocatorheap_base.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "test_i_camstream_common.h"
#include "test_i_camstream_network.h"
#include "test_i_connection_manager_common.h"
#include "test_i_source_eventhandler.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IAMStreamConfig;
struct ISampleGrabber;

struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_DirectShow_ConnectionState;
struct Test_I_Source_DirectShow_StreamConfiguration;
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_MediaFoundation_ConnectionState;
struct Test_I_Source_MediaFoundation_StreamConfiguration;
#else
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_V4L2_ConnectionState;
struct Test_I_Source_V4L2_StreamConfiguration;
#endif
template <typename ConfigurationType>
class Test_I_Source_SignalHandler_T;
template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
class Test_I_Source_EventHandler_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_UserData
 : Stream_UserData
{
  inline Test_I_Source_DirectShow_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_Source_ConnectionConfiguration*        configuration;
  Test_I_Source_DirectShow_StreamConfiguration* streamConfiguration;
};
struct Test_I_Source_MediaFoundation_UserData
 : Stream_UserData
{
  inline Test_I_Source_MediaFoundation_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_Source_ConnectionConfiguration*             configuration;
  Test_I_Source_MediaFoundation_StreamConfiguration* streamConfiguration;
};
#else
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_V4L2_UserData
 : Stream_UserData
{
  inline Test_I_Source_V4L2_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  struct Test_I_Source_ConnectionConfiguration*  configuration;
  struct Test_I_Source_V4L2_StreamConfiguration* streamConfiguration;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Source_DirectShow_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Test_I_Source_DirectShow_UserData* userData;
};
struct Test_I_Source_MediaFoundation_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Source_MediaFoundation_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Test_I_Source_MediaFoundation_UserData* userData;
};
#else
struct Test_I_Source_V4L2_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Source_V4L2_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Test_I_Source_V4L2_UserData* userData;
};
#endif

struct Test_I_Source_Stream_StatisticData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamState;
struct Test_I_Source_DirectShow_ModuleHandlerConfiguration;
class Test_I_Source_DirectShow_Stream_Message;
class Test_I_Source_DirectShow_Stream_SessionMessage;

struct Test_I_Source_MediaFoundation_StreamState;
struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration;
class Test_I_Source_MediaFoundation_Stream_Message;
class Test_I_Source_MediaFoundation_Stream_SessionMessage;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_I_Source_DirectShow_Stream_Message,
                                Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_DirectShow_ControlMessage_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_DirectShow_StreamState,
                      struct Test_I_Source_DirectShow_StreamConfiguration,
                      struct Test_I_Source_Stream_StatisticData,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                      struct Test_I_Source_DirectShow_SessionData,
                      Test_I_Source_DirectShow_SessionData_t,
                      Test_I_DirectShow_ControlMessage_t,
                      Test_I_Source_DirectShow_Stream_Message,
                      Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_Source_DirectShow_StreamBase_t;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_I_Source_MediaFoundation_Stream_Message,
                                Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_MediaFoundation_ControlMessage_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_MediaFoundation_StreamState,
                      struct Test_I_Source_MediaFoundation_StreamConfiguration,
                      struct Test_I_Source_Stream_StatisticData,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                      struct Test_I_Source_MediaFoundation_SessionData,
                      Test_I_Source_MediaFoundation_SessionData_t,
                      Test_I_MediaFoundation_ControlMessage_t,
                      Test_I_Source_MediaFoundation_Stream_Message,
                      Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_Source_MediaFoundation_StreamBase_t;
#else
struct Test_I_Source_V4L2_StreamState;
struct Test_I_Source_V4L2_ModuleHandlerConfiguration;
class Test_I_Source_V4L2_Stream_Message;
class Test_I_Source_V4L2_Stream_SessionMessage;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_I_Source_V4L2_Stream_Message,
                                Test_I_Source_V4L2_Stream_SessionMessage> Test_I_V4L2_ControlMessage_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_V4L2_StreamState,
                      struct Test_I_Source_V4L2_StreamConfiguration,
                      struct Test_I_Source_Stream_StatisticData,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                      struct Test_I_Source_V4L2_SessionData,
                      Test_I_Source_V4L2_SessionData_t,
                      Test_I_V4L2_ControlMessage_t,
                      Test_I_Source_V4L2_Stream_Message,
                      Test_I_Source_V4L2_Stream_SessionMessage> Test_I_Source_V4L2_StreamBase_t;
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  inline Test_I_Source_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , builder (NULL)
   , connection (NULL)
   , connectionManager (NULL)
   , contextID (0)
   , device ()
   , format (NULL)
   , socketHandlerConfiguration (NULL)
   //, statisticCollectionInterval (ACE_Time_Value::zero)
   , stream (NULL)
   , windowController (NULL)
  {
    format =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    if (!format)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing\n")));
    } // end IF
    else
      ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
  };

  struct tagRECT                                              area; // visualization module
  IGraphBuilder*                                              builder;
  Test_I_Source_DirectShow_IConnection_t*                     connection; // TCP target/IO module
  Test_I_Source_DirectShow_InetConnectionManager_t*           connectionManager; // TCP IO module
  guint                                                       contextID;
  // *NOTE*: "FriendlyName" property
  std::string                                                 device; // source module
  struct _AMMediaType*                                        format; // source module
  struct Test_I_Source_DirectShow_SocketHandlerConfiguration* socketHandlerConfiguration;
  //ACE_Time_Value                                            statisticCollectionInterval;
  Test_I_Source_DirectShow_StreamBase_t*                      stream;
  IVideoWindow*                                               windowController; // visualization module
};
struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  inline Test_I_Source_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , connection (NULL)
   , connectionManager (NULL)
   , contextID (0)
   , device ()
   , format (NULL)
   , mediaSource (NULL)
   , sampleGrabberNodeId (0)
   , session (NULL)
   , socketHandlerConfiguration (NULL)
   //, statisticCollectionInterval (ACE_Time_Value::zero)
   , stream (NULL)
   , windowController (NULL)
  {
    HRESULT result = MFCreateMediaType (&format);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  };

  struct tagRECT                                                   area;
  Test_I_Source_MediaFoundation_IConnection_t*                     connection; // TCP target/IO module
  Test_I_Source_MediaFoundation_InetConnectionManager_t*           connectionManager; // TCP IO module
  guint                                                            contextID;
  // *NOTE*: "FriendlyName" property
  std::string                                                      device;
  TOPOID                                                           sampleGrabberNodeId;
  struct Test_I_Source_MediaFoundation_SocketHandlerConfiguration* socketHandlerConfiguration;
  IMFMediaType*                                                    format;
  IMFMediaSource*                                                  mediaSource;
  IMFMediaSession*                                                 session;
  //ACE_Time_Value                                          statisticCollectionInterval;
  Test_I_Source_MediaFoundation_StreamBase_t*                      stream;
  IMFVideoDisplayControl*                                          windowController;
};
#else

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_V4L2_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_Stream_SessionMessage> Test_I_Source_V4L2_ISessionNotify_t;
typedef std::list<Test_I_Source_V4L2_ISessionNotify_t*> Test_I_Source_V4L2_Subscribers_t;
typedef Test_I_Source_V4L2_Subscribers_t::iterator Test_I_Source_V4L2_SubscribersIterator_t;
struct Test_I_Source_V4L2_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  inline Test_I_Source_V4L2_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , buffers (MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS)
   , connection (NULL)
   , connectionManager (NULL)
   , device ()
   , fileDescriptor (-1)
   , format ()
   , frameRate ()
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , socketHandlerConfiguration (NULL)
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , stream (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , v4l2Window (NULL)
  {
    ACE_OS::memset (&format, 0, sizeof (format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ACE_OS::memset (&frameRate, 0, sizeof (frameRate));
  };

  GdkRectangle                                          area;
  __u32                                                 buffers; // v4l device buffers
  Test_I_Source_V4L2_IConnection_t*                     connection; // TCP target/IO module
  Test_I_Source_V4L2_InetConnectionManager_t*           connectionManager; // TCP IO module
  // *NOTE*: v4l2 device file (e.g. "/dev/video0" (Linux))
  std::string                                           device;
  int                                                   fileDescriptor;
  struct v4l2_format                                    format;
  struct v4l2_fract                                     frameRate; // time-per-frame (s)
  v4l2_memory                                           method; // v4l camera source
  struct Test_I_Source_V4L2_SocketHandlerConfiguration* socketHandlerConfiguration;
  ACE_Time_Value                                        statisticCollectionInterval;
  Test_I_Source_V4L2_StreamBase_t*                      stream;
  Test_I_Source_V4L2_ISessionNotify_t*                  subscriber;
  Test_I_Source_V4L2_Subscribers_t*                     subscribers;
  struct v4l2_window*                                   v4l2Window;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_I_Source_DirectShow_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , connectionManager (NULL)
//   , statisticReportingInterval (0)
   , stream (NULL)
  {};

  Test_I_Source_DirectShow_InetConnectionManager_t* connectionManager;
//  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_DirectShow_StreamBase_t*            stream;
};
typedef Test_I_Source_SignalHandler_T<Test_I_Source_DirectShow_SignalHandlerConfiguration> Test_I_Source_DirectShow_SignalHandler_t;
struct Test_I_Source_MediaFoundation_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_I_Source_MediaFoundation_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   //   , statisticReportingInterval (0)
   , stream (NULL)
  {};

  Test_I_Source_MediaFoundation_InetConnectionManager_t* connectionManager;
  //  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_MediaFoundation_StreamBase_t*            stream;
};
typedef Test_I_Source_SignalHandler_T<Test_I_Source_MediaFoundation_SignalHandlerConfiguration> Test_I_Source_MediaFoundation_SignalHandler_t;
#else
struct Test_I_Source_V4L2_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_I_Source_V4L2_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   //   , statisticReportingInterval (0)
   , stream (NULL)
  {};

  Test_I_Source_V4L2_InetConnectionManager_t* connectionManager;
  //  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_V4L2_StreamBase_t*            stream;
};
typedef Test_I_Source_SignalHandler_T<Test_I_Source_V4L2_SignalHandlerConfiguration> Test_I_Source_V4L2_SignalHandler_t;
#endif

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_SessionData
 : Test_I_CamStream_DirectShow_SessionData
{
  inline Test_I_Source_DirectShow_SessionData ()
   : Test_I_CamStream_DirectShow_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};

  inline Test_I_Source_DirectShow_SessionData& operator+= (const Test_I_Source_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_CamStream_DirectShow_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState
                                       : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Source_DirectShow_ConnectionState* connectionState;
  struct Test_I_Source_DirectShow_UserData*        userData;
};
typedef Stream_SessionData_T<Test_I_Source_DirectShow_SessionData> Test_I_Source_DirectShow_SessionData_t;
struct Test_I_Source_MediaFoundation_SessionData
 : Test_I_CamStream_MediaFoundation_SessionData
{
  inline Test_I_Source_MediaFoundation_SessionData ()
   : Test_I_CamStream_MediaFoundation_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};

  inline Test_I_Source_MediaFoundation_SessionData& operator+= (const Test_I_Source_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_CamStream_MediaFoundation_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState
                                       : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Source_MediaFoundation_ConnectionState* connectionState;
  struct Test_I_Source_MediaFoundation_UserData*        userData;
};
typedef Stream_SessionData_T<Test_I_Source_MediaFoundation_SessionData> Test_I_Source_MediaFoundation_SessionData_t;
#else
struct Test_I_Source_V4L2_SessionData
 : Test_I_CamStream_V4L2_SessionData
{
  inline Test_I_Source_V4L2_SessionData ()
   : Test_I_CamStream_V4L2_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};

  inline Test_I_Source_V4L2_SessionData& operator+= (const Test_I_Source_V4L2_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_CamStream_V4L2_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState
                                       : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Source_V4L2_ConnectionState* connectionState;
  struct Test_I_Source_V4L2_UserData*        userData;
};
typedef Stream_SessionData_T<Test_I_Source_V4L2_SessionData> Test_I_Source_V4L2_SessionData_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Source_DirectShow_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
  {};

  // **************************** stream data **********************************
  struct Test_I_Source_DirectShow_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
struct Test_I_MediaFoundationConfiguration;
struct Test_I_Source_MediaFoundation_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Source_MediaFoundation_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , mediaFoundationConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration*                      mediaFoundationConfiguration;
  // **************************** stream data **********************************
  struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
#else
struct Test_I_Source_V4L2_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Source_V4L2_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
  {};

  // **************************** stream data **********************************
  struct Test_I_Source_V4L2_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamState
 : Stream_State
{
  inline Test_I_Source_DirectShow_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_DirectShow_SessionData* currentSessionData;
  struct Test_I_Source_DirectShow_UserData*    userData;
};
struct Test_I_Source_MediaFoundation_StreamState
 : Stream_State
{
  inline Test_I_Source_MediaFoundation_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_MediaFoundation_SessionData* currentSessionData;
  struct Test_I_Source_MediaFoundation_UserData*    userData;
};
#else
struct Test_I_Source_V4L2_StreamState
 : Stream_State
{
  inline Test_I_Source_V4L2_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_V4L2_SessionData* currentSessionData;
  struct Test_I_Source_V4L2_UserData*    userData;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_Configuration
 : Test_I_CamStream_Configuration
{
  inline Test_I_Source_DirectShow_Configuration ()
   : Test_I_CamStream_Configuration ()
   , signalHandlerConfiguration ()
   , socketHandlerConfiguration ()
   , connectionConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** signal data **********************************
  struct Test_I_Source_DirectShow_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Test_I_Source_DirectShow_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_I_Source_ConnectionConfiguration               connectionConfiguration;
  // **************************** stream data **********************************
  struct Test_I_Source_DirectShow_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_Source_DirectShow_StreamConfiguration        streamConfiguration;

  struct Test_I_Source_DirectShow_UserData                   userData;
};
struct Test_I_Source_MediaFoundation_Configuration
 : Test_I_CamStream_Configuration
{
  inline Test_I_Source_MediaFoundation_Configuration ()
    : Test_I_CamStream_Configuration ()
    , mediaFoundationConfiguration ()
    , signalHandlerConfiguration ()
    , socketHandlerConfiguration ()
    , connectionConfiguration ()
    , moduleHandlerConfiguration ()
    , streamConfiguration ()
    , userData ()
  {};

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration                      mediaFoundationConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Source_MediaFoundation_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Test_I_Source_MediaFoundation_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_I_Source_ConnectionConfiguration                    connectionConfiguration;
  // **************************** stream data **********************************
  struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_Source_MediaFoundation_StreamConfiguration        streamConfiguration;

  struct Test_I_Source_MediaFoundation_UserData                   userData;
};
#else
struct Test_I_Source_V4L2_Configuration
  : Test_I_CamStream_Configuration
{
  inline Test_I_Source_V4L2_Configuration ()
    : Test_I_CamStream_Configuration ()
    , signalHandlerConfiguration ()
    , socketHandlerConfiguration ()
    , connectionConfiguration ()
    , moduleHandlerConfiguration ()
    , streamConfiguration ()
    , userData ()
  {};

  // **************************** signal data **********************************
  struct Test_I_Source_V4L2_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Test_I_Source_V4L2_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_I_Source_ConnectionConfiguration         connectionConfiguration;
  // **************************** stream data **********************************
  struct Test_I_Source_V4L2_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_Source_V4L2_StreamConfiguration        streamConfiguration;

  struct Test_I_Source_V4L2_UserData                   userData;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_ControlMessage_T<ENUM Stream_ControlMessageType,
//                                struct Stream_AllocatorConfiguration,
//                                Test_I_Source_DirectShow_Stream_Message,
//                                Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_DirectShow_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_I_DirectShow_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_Source_DirectShow_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_DirectShow_Stream_Message,
                                    Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_Source_DirectShow_ISessionNotify_t;
struct Test_I_Source_DirectShow_GTK_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Source_DirectShow_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_DirectShow_Stream_Message,
                                     Test_I_Source_DirectShow_Stream_SessionMessage,
                                     struct Test_I_Source_DirectShow_GTK_CBData> Test_I_Source_DirectShow_EventHandler_t;
typedef std::list<Test_I_Source_DirectShow_ISessionNotify_t*> Test_I_Source_DirectShow_Subscribers_t;
typedef Test_I_Source_DirectShow_Subscribers_t::iterator Test_I_Source_DirectShow_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_I_Source_DirectShow_ISessionNotify_t> Test_I_Source_DirectShow_ISubscribe_t;

typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_I_Source_MediaFoundation_Stream_Message,
                                Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_MediaFoundation_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_I_MediaFoundation_ControlMessage_t,
                                          Test_I_Source_MediaFoundation_Stream_Message,
                                          Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_Source_MediaFoundation_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_MediaFoundation_Stream_Message,
                                    Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_Source_MediaFoundation_ISessionNotify_t;
struct Test_I_Source_MediaFoundation_GTK_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Source_MediaFoundation_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_MediaFoundation_Stream_Message,
                                     Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                     Test_I_Source_MediaFoundation_GTK_CBData> Test_I_Source_MediaFoundation_EventHandler_t;
typedef std::list<Test_I_Source_MediaFoundation_ISessionNotify_t*> Test_I_Source_MediaFoundation_Subscribers_t;
typedef Test_I_Source_MediaFoundation_Subscribers_t::iterator Test_I_Source_MediaFoundation_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_I_Source_MediaFoundation_ISessionNotify_t> Test_I_Source_MediaFoundation_ISubscribe_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_I_V4L2_ControlMessage_t,
                                          Test_I_Source_V4L2_Stream_Message,
                                          Test_I_Source_V4L2_Stream_SessionMessage> Test_I_Source_V4L2_MessageAllocator_t;

struct Test_I_Source_V4L2_GTK_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Source_V4L2_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_V4L2_Stream_Message,
                                     Test_I_Source_V4L2_Stream_SessionMessage,
                                     Test_I_Source_V4L2_GTK_CBData> Test_I_Source_V4L2_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_Source_V4L2_ISessionNotify_t> Test_I_Source_V4L2_ISubscribe_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_GTK_CBData
 : Test_I_CamStream_GTK_CBData
{
  inline Test_I_Source_DirectShow_GTK_CBData ()
   : Test_I_CamStream_GTK_CBData ()
   , configuration (NULL)
   , progressData ()
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
   , streamConfiguration (NULL)
   , UDPStream (NULL)
  {
    progressData.GTKState = this;
  };

  struct Test_I_Source_DirectShow_Configuration* configuration;
  struct Test_I_CamStream_GTK_ProgressData       progressData;
  Test_I_Source_DirectShow_StreamBase_t*         stream;
  Test_I_Source_DirectShow_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                      subscribersLock;
  IAMStreamConfig*                               streamConfiguration;
  Test_I_Source_DirectShow_StreamBase_t*         UDPStream;
};
struct Test_I_Source_MediaFoundation_GTK_CBData
 : Test_I_CamStream_GTK_CBData
{
  inline Test_I_Source_MediaFoundation_GTK_CBData ()
   : Test_I_CamStream_GTK_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
   , UDPStream (NULL)
  {};

  struct Test_I_Source_MediaFoundation_Configuration* configuration;
  Test_I_Source_MediaFoundation_StreamBase_t*         stream;
  Test_I_Source_MediaFoundation_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                           subscribersLock;
  Test_I_Source_MediaFoundation_StreamBase_t*         UDPStream;
};
#else
struct Test_I_Source_V4L2_GTK_CBData
 : Test_I_CamStream_GTK_CBData
{
  inline Test_I_Source_V4L2_GTK_CBData ()
   : Test_I_CamStream_GTK_CBData ()
   , configuration (NULL)
   , device (-1)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
   , UDPStream (NULL)
  {};

  struct Test_I_Source_V4L2_Configuration* configuration;
  int                                      device; // (capture) device file descriptor
  Test_I_Source_V4L2_StreamBase_t*         stream;
  Test_I_Source_V4L2_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                subscribersLock;
  Test_I_Source_V4L2_StreamBase_t*         UDPStream;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_ThreadData
 : Test_I_CamStream_ThreadData
{
  inline Test_I_Source_DirectShow_ThreadData ()
   : Test_I_CamStream_ThreadData ()
   , CBData (NULL)
  {};

  struct Test_I_Source_DirectShow_GTK_CBData* CBData;
};
struct Test_I_Source_MediaFoundation_ThreadData
 : Test_I_CamStream_ThreadData
{
  inline Test_I_Source_MediaFoundation_ThreadData ()
   : Test_I_CamStream_ThreadData ()
   , CBData (NULL)
  {};

  struct Test_I_Source_MediaFoundation_GTK_CBData* CBData;
};
#else
struct Test_I_Source_V4L2_ThreadData
 : Test_I_CamStream_ThreadData
{
  inline Test_I_Source_V4L2_ThreadData ()
   : Test_I_CamStream_ThreadData ()
   , CBData (NULL)
  {};

  struct Test_I_Source_V4L2_GTK_CBData* CBData;
};
#endif

#endif
