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

#ifndef TEST_I_TARGET_COMMON_H
#define TEST_I_TARGET_COMMON_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/os_include/sys/os_socket.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <d3d9.h>
#include <evr.h>
#include <mfapi.h>
//#include <mfidl.h>
//#include <mmeapi.h>
//#include <mtype.h>
#include <strmif.h>
#else
//#include <linux/videodev2.h>
#endif

#include "gtk/gtk.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_control_message.h"

#include "stream_dec_defines.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"

#include "net_defines.h"
#include "net_ilistener.h"

#include "test_i_defines.h"

#include "test_i_camstream_common.h"
#include "test_i_camstream_network.h"
#include "test_i_connection_manager_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Target_DirectShow_Stream_Message;
class Test_I_Target_DirectShow_Stream_SessionMessage;
class Test_I_Target_MediaFoundation_Stream_Message;
class Test_I_Target_MediaFoundation_Stream_SessionMessage;
#else
struct v4l2_window;
class Test_I_Target_Stream_Message;
class Test_I_Target_Stream_SessionMessage;
#endif
template <typename ConfigurationType,
          typename ConnectionManagerType>
class Test_I_Target_SignalHandler_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_MessageData
{
  inline Test_I_Target_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0.0)
  {};

  IMediaSample* sample;
  double        sampleTime;
};
struct Test_I_Target_MediaFoundation_MessageData
{
  inline Test_I_Target_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMFMediaBuffer* sample;
  LONGLONG        sampleTime;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_ConnectionConfiguration;
struct Test_I_Target_DirectShow_StreamConfiguration;
struct Test_I_Target_DirectShow_UserData
 : Stream_UserData
{
  inline Test_I_Target_DirectShow_UserData ()
   : Stream_UserData ()
   , connectionConfiguration (NULL)
   , streamConfiguration (NULL)
  {};

  struct Test_I_Target_DirectShow_ConnectionConfiguration* connectionConfiguration;
  struct Test_I_Target_DirectShow_StreamConfiguration*     streamConfiguration;
};
struct Test_I_Target_MediaFoundation_ConnectionConfiguration;
struct Test_I_Target_MediaFoundation_StreamConfiguration;
struct Test_I_Target_MediaFoundation_UserData
 : Stream_UserData
{
  inline Test_I_Target_MediaFoundation_UserData ()
   : Stream_UserData ()
   , connectionConfiguration (NULL)
   , streamConfiguration (NULL)
  {};

  struct Test_I_Target_MediaFoundation_ConnectionConfiguration* connectionConfiguration;
  struct Test_I_Target_MediaFoundation_StreamConfiguration*     streamConfiguration;
};
#else
struct Test_I_Target_ConnectionConfiguration;
struct Test_I_Target_UserData
 : Stream_UserData
{
  inline Test_I_Target_UserData ()
   : Stream_UserData ()
  {};
};
#endif

struct Test_I_Target_ConnectionState;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//struct Test_I_Target_SessionData
// : Test_I_CamStream_DirectShow_SessionData
//{
//  inline Test_I_Target_SessionData ()
//   : Test_I_CamStream_DirectShow_SessionData ()
//   , connectionState (NULL)
//   , targetFileName ()
//   , userData (NULL)
//  {};
//
//  inline struct Test_I_Target_SessionData& operator+= (const struct Test_I_Target_SessionData& rhs_in)
//  {
//    // *NOTE*: the idea is to 'merge' the data
//    Test_I_CamStream_DirectShow_SessionData::operator+= (rhs_in);
//
//    connectionState =
//      (connectionState ? connectionState : rhs_in.connectionState);
//    targetFileName =
//      (targetFileName.empty () ? rhs_in.targetFileName : targetFileName);
//    userData = (userData ? userData : rhs_in.userData);
//
//    return *this;
//  }
//
//  struct Test_I_Target_ConnectionState* connectionState;
//  std::string                           targetFileName;
//  struct Test_I_Target_UserData*        userData;
//};
//typedef Stream_SessionData_T<struct Test_I_Target_SessionData> Test_I_Target_SessionData_t;
struct Test_I_Target_DirectShow_SessionData
 : Test_I_CamStream_DirectShow_SessionData
{
  inline Test_I_Target_DirectShow_SessionData ()
   : Test_I_CamStream_DirectShow_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
   , windowController (NULL)
  {};

  inline struct Test_I_Target_DirectShow_SessionData& operator+= (const struct Test_I_Target_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_CamStream_DirectShow_SessionData::operator+= (rhs_in);

    connectionState =
      (connectionState ? connectionState : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Target_DirectShow_ConnectionState* connectionState;
  struct Test_I_Target_DirectShow_UserData*        userData;
  IVideoWindow*                                    windowController;
};
typedef Stream_SessionData_T<struct Test_I_Target_DirectShow_SessionData> Test_I_Target_DirectShow_SessionData_t;
struct Test_I_Target_MediaFoundation_SessionData
 : Test_I_CamStream_MediaFoundation_SessionData
{
  inline Test_I_Target_MediaFoundation_SessionData ()
   : Test_I_CamStream_MediaFoundation_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};

  inline struct Test_I_Target_MediaFoundation_SessionData& operator+= (const struct Test_I_Target_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_CamStream_MediaFoundation_SessionData::operator+= (rhs_in);

    connectionState =
      (connectionState ? connectionState : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Target_MediaFoundation_ConnectionState* connectionState;
  struct Test_I_Target_MediaFoundation_UserData*        userData;
};
typedef Stream_SessionData_T<struct Test_I_Target_MediaFoundation_SessionData> Test_I_Target_MediaFoundation_SessionData_t;
#else
struct Test_I_Target_SessionData
 : Test_I_CamStream_V4L2_SessionData
{
  inline Test_I_Target_SessionData ()
   : Test_I_CamStream_V4L2_SessionData ()
   , connectionState (NULL)
   , format (AV_PIX_FMT_RGB24)
   , height (0)
   , targetFileName ()
   , width (0)
   , userData (NULL)
  {};

  inline struct Test_I_Target_SessionData& operator+= (const struct Test_I_Target_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_CamStream_V4L2_SessionData::operator+= (rhs_in);

    connectionState =
      (connectionState ? connectionState : rhs_in.connectionState);
    targetFileName =
      (targetFileName.empty () ? rhs_in.targetFileName : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Target_ConnectionState* connectionState;
  enum AVPixelFormat                    format;
  unsigned int                          height;
  std::string                           targetFileName;
  unsigned int                          width;

  struct Test_I_Target_UserData*        userData;
};
typedef Stream_SessionData_T<struct Test_I_Target_SessionData> Test_I_Target_SessionData_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  inline Test_I_Target_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   //, format (NULL)
   , module (NULL)
   , pinConfiguration (NULL)
  {};

  // *TODO*: specify this as part of the network protocol header/handshake
  //struct _AMMediaType*                                           format; // handle
  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Target_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Target_DirectShow_Stream_Message,
                                    Test_I_Target_DirectShow_Stream_SessionMessage> Test_I_Target_DirectShow_ISessionNotify_t;
typedef std::list<Test_I_Target_DirectShow_ISessionNotify_t*> Test_I_Target_DirectShow_Subscribers_t;
typedef Test_I_Target_DirectShow_Subscribers_t::iterator Test_I_Target_DirectShow_SubscribersIterator_t;
struct Test_I_Target_DirectShow_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  inline Test_I_Target_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , contextId (0)
   , crunch (true)
   , device ()
   , filterCLSID ()
   , filterConfiguration (NULL)
   , format (NULL)
   , graphBuilder (NULL)
   , push (MODULE_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , queue (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , window (NULL)
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

  struct tagRECT                                       area;              // display module
  Test_I_Target_DirectShow_IConnection_t*              connection;        // Net source/IO module
  Test_I_Target_DirectShow_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_Target_DirectShow_InetConnectionManager_t*    connectionManager; // Net IO module
  guint                                                contextId;
  bool                                                 crunch;            // splitter module
  std::string                                          device; // FriendlyName
  struct Test_I_Target_DirectShow_FilterConfiguration* filterConfiguration;
  struct _GUID                                         filterCLSID;
  struct _AMMediaType*                                 format;            // splitter module
  IGraphBuilder*                                       graphBuilder;      // display module
  bool                                                 push; // media sample passing strategy
  ACE_Message_Queue_Base*                              queue; // (inbound) buffer queue handle
  Test_I_Target_DirectShow_StreamConfiguration_t*      streamConfiguration;
  Test_I_Target_DirectShow_ISessionNotify_t*           subscriber;        // event handler module
  Test_I_Target_DirectShow_Subscribers_t*              subscribers;       // event handler module
  HWND                                                 window;            // display module
  IVideoWindow*                                        windowController;  // display module
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Target_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Target_MediaFoundation_Stream_Message,
                                    Test_I_Target_MediaFoundation_Stream_SessionMessage> Test_I_Target_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_I_Target_MediaFoundation_ISessionNotify_t*> Test_I_Target_MediaFoundation_Subscribers_t;
typedef Test_I_Target_MediaFoundation_Subscribers_t::iterator Test_I_Target_MediaFoundation_SubscribersIterator_t;
struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  inline Test_I_Target_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , contextId (0)
   , crunch (true)
   , device ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , format (NULL)
   , mediaSource (NULL)
   , queue (NULL)
   //, sourceReader (NULL)
   , rendererNodeId (0)
   , session (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , window (NULL)
   , windowController (NULL)
  {
    HRESULT result = MFCreateMediaType (&format);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  };

  struct tagRECT                                            area;                      // display module
  Test_I_Target_MediaFoundation_IConnection_t*              connection;                // net source/IO module
  Test_I_Target_MediaFoundation_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_Target_MediaFoundation_InetConnectionManager_t*    connectionManager;         // net IO module
  guint                                                     contextId;
  bool                                                      crunch;                    // splitter module
  std::string                                               device; // FriendlyName
  IDirect3DDevice9Ex*                                       direct3DDevice;            // display module
  UINT                                                      direct3DManagerResetToken; // display module
  IMFMediaType*                                             format;                    // display module
  IMFMediaSource*                                           mediaSource;
  ACE_Message_Queue_Base*                                   queue; // (inbound) buffer queue handle
  TOPOID                                                    rendererNodeId;            // display module
  //IMFSourceReaderEx*                                             sourceReader;
  IMFMediaSession*                                          session;
  Test_I_Target_MediaFoundation_StreamConfiguration_t*      streamConfiguration;
  Test_I_Target_MediaFoundation_ISessionNotify_t*           subscriber;                // event handler module
  Test_I_Target_MediaFoundation_Subscribers_t*              subscribers;               // event handler module
  HWND                                                      window;                    // display module
  IMFVideoDisplayControl*                                   windowController;          // display module
};
#else
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Target_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Target_Stream_Message,
                                    Test_I_Target_Stream_SessionMessage> Test_I_Target_ISessionNotify_t;
typedef std::list<Test_I_Target_ISessionNotify_t*> Test_I_Target_Subscribers_t;
typedef Test_I_Target_Subscribers_t::iterator Test_I_Target_SubscribersIterator_t;
struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  inline Test_I_Target_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , contextID (0)
   , crunch (false)
   , format (AV_PIX_FMT_RGB24)
   , height (0)
   , queue (NULL)
   , sourceFormat ()
   , streamConfiguration (NULL)
   , targetFileName ()
   , subscriber (NULL)
   , subscribers (NULL)
   , v4l2Format ()
   , v4l2Window ()
   , width (0)
   , window (NULL)
  {};

  GdkRectangle                              area;
  Test_I_Target_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_Target_InetConnectionManager_t*    connectionManager; // net IO module
  guint                                     contextID;
  bool                                      crunch;            // splitter module
  enum AVPixelFormat                        format;
  unsigned int                              height;
  ACE_Message_Queue_Base*                   queue;  // (inbound) buffer queue handle
  GdkRectangle                              sourceFormat; // gtk pixbuf module
  // *TODO*: remove this ASAP
  Test_I_Target_StreamConfiguration_t*      streamConfiguration;
  std::string                               targetFileName;    // file writer module
  Test_I_Target_ISessionNotify_t*           subscriber;
  Test_I_Target_Subscribers_t*              subscribers;
  struct v4l2_format                        v4l2Format;        // splitter module
  struct v4l2_window                        v4l2Window;
  unsigned int                              width;
  GdkWindow*                                window;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_ListenerConfiguration
 : Net_ListenerConfiguration
{
  inline Test_I_Target_DirectShow_ListenerConfiguration ()
   : Net_ListenerConfiguration ()
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
  {
    //socketHandlerConfiguration.socketConfiguration.address.set_port_number (TEST_I_DEFAULT_PORT,
    //                                                                        1);
  };

  Test_I_Target_DirectShow_InetConnectionManager_t*          connectionManager;
  struct Test_I_Target_DirectShow_SocketHandlerConfiguration socketHandlerConfiguration;
  ACE_Time_Value                                             statisticReportingInterval; // [ACE_Time_Value::zero: off]
};
typedef Net_IListener_T<struct Test_I_Target_DirectShow_ListenerConfiguration,
                        struct Test_I_Target_DirectShow_SocketHandlerConfiguration> Test_I_Target_DirectShow_IListener_t;
struct Test_I_Target_MediaFoundation_ListenerConfiguration
 : Net_ListenerConfiguration
{
  inline Test_I_Target_MediaFoundation_ListenerConfiguration ()
   : Net_ListenerConfiguration ()
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
  {
    //socketHandlerConfiguration.socketConfiguration.address.set_port_number (TEST_I_DEFAULT_PORT,
                                                                              //1);
  };

  Test_I_Target_MediaFoundation_InetConnectionManager_t*          connectionManager;
  struct Test_I_Target_MediaFoundation_SocketHandlerConfiguration socketHandlerConfiguration;
  ACE_Time_Value                                                  statisticReportingInterval; // [ACE_Time_Value::zero: off]
};
typedef Net_IListener_T<struct Test_I_Target_MediaFoundation_ListenerConfiguration,
                        struct Test_I_Target_MediaFoundation_SocketHandlerConfiguration> Test_I_Target_MediaFoundation_IListener_t;
#else
struct Test_I_Target_ListenerConfiguration
 : Net_ListenerConfiguration
{
  inline Test_I_Target_ListenerConfiguration ()
   : Net_ListenerConfiguration ()
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
  {
    //address.set_port_number (TEST_I_DEFAULT_PORT, 1);
  };

  Test_I_Target_InetConnectionManager_t*          connectionManager;
  struct Test_I_Target_SocketHandlerConfiguration socketHandlerConfiguration;
  ACE_Time_Value                                  statisticReportingInterval; // [ACE_Time_Value::zero: off]
};
typedef Net_IListener_T<struct Test_I_Target_ListenerConfiguration,
                        struct Test_I_Target_SocketHandlerConfiguration> Test_I_Target_IListener_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  inline Test_I_Target_DirectShow_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   , listener (NULL)
   , statisticReportingHandler (NULL)
  {};

  Net_IConnectionManagerBase*           connectionManager;
  Test_I_Target_DirectShow_IListener_t* listener;
  Test_I_StatisticReportingHandler_t*   statisticReportingHandler;
};
typedef Test_I_Target_SignalHandler_T<struct Test_I_Target_DirectShow_SignalHandlerConfiguration,
                                      Test_I_Target_DirectShow_InetConnectionManager_t> Test_I_Target_DirectShow_SignalHandler_t;
struct Test_I_Target_MediaFoundation_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  inline Test_I_Target_MediaFoundation_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   , listener (NULL)
   , statisticReportingHandler (NULL)
  {};

  Net_IConnectionManagerBase*                connectionManager;
  Test_I_Target_MediaFoundation_IListener_t* listener;
  Test_I_StatisticReportingHandler_t*        statisticReportingHandler;
};
typedef Test_I_Target_SignalHandler_T<struct Test_I_Target_MediaFoundation_SignalHandlerConfiguration,
                                      Test_I_Target_MediaFoundation_InetConnectionManager_t> Test_I_Target_MediaFoundation_SignalHandler_t;
#else
struct Test_I_Target_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  inline Test_I_Target_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerID (-1)
  {};

  Test_I_Target_InetConnectionManager_t* connectionManager;
  Test_I_Target_IListener_t*             listener;
  Test_I_StatisticReportingHandler_t*    statisticReportingHandler;
  long                                   statisticReportingTimerID;
};
typedef Test_I_Target_SignalHandler_T<struct Test_I_Target_SignalHandlerConfiguration,
                                      Test_I_Target_InetConnectionManager_t> Test_I_Target_SignalHandler_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Target_DirectShow_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , graphBuilder (NULL)
   , userData (NULL)
  {};

  IGraphBuilder*                            graphBuilder;

  struct Test_I_Target_DirectShow_UserData* userData;
};
struct Test_I_Target_MediaFoundation_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Target_MediaFoundation_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , userData (NULL)
  {};

  struct Test_I_Target_MediaFoundation_UserData* userData;
};
#else
struct Test_I_Target_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Target_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , window (NULL)
   , userData (NULL)
  {};

  GdkWindow*                     window;

  struct Test_I_Target_UserData* userData;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_StreamState
 : Stream_State
{
  inline Test_I_Target_DirectShow_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Target_DirectShow_SessionData* currentSessionData;
  struct Test_I_Target_DirectShow_UserData*    userData;
};
struct Test_I_Target_MediaFoundation_StreamState
 : Stream_State
{
  inline Test_I_Target_MediaFoundation_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Target_MediaFoundation_SessionData* currentSessionData;
  struct Test_I_Target_MediaFoundation_UserData*    userData;
};
#else
struct Test_I_Target_StreamState
 : Stream_State
{
  inline Test_I_Target_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Target_SessionData* currentSessionData;
  struct Test_I_Target_UserData*    userData;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_Configuration
 : Test_I_CamStream_Configuration
{
  inline Test_I_Target_DirectShow_Configuration ()
   : Test_I_CamStream_Configuration ()
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
   , pinConfiguration ()
   , filterConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** socket data **********************************
  Test_I_Target_DirectShow_ConnectionConfigurations_t            connectionConfigurations;
  // **************************** listener data ********************************
  ACE_HANDLE                                                     handle;
  //Test_I_Target_IListener_t*               listener;
  struct Test_I_Target_DirectShow_ListenerConfiguration          listenerConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Target_DirectShow_SignalHandlerConfiguration     signalHandlerConfiguration;
  // **************************** stream data **********************************
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration pinConfiguration;
  struct Test_I_Target_DirectShow_FilterConfiguration            filterConfiguration;
  Test_I_Target_DirectShow_StreamConfiguration_t                 streamConfiguration;

  struct Test_I_Target_DirectShow_UserData                       userData;
};
struct Test_I_Target_MediaFoundation_Configuration
 : Test_I_CamStream_Configuration
{
  inline Test_I_Target_MediaFoundation_Configuration ()
   : Test_I_CamStream_Configuration ()
   , mediaFoundationConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   , listenerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration                      mediaFoundationConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Target_MediaFoundation_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_Target_MediaFoundation_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** listener data ********************************
  ACE_HANDLE                                                      handle;
  //Test_I_Target_IListener_t*                             listener;
  struct Test_I_Target_MediaFoundation_ListenerConfiguration      listenerConfiguration;
  // **************************** stream data **********************************
  Test_I_Target_MediaFoundation_StreamConfiguration_t             streamConfiguration;

  struct Test_I_Target_MediaFoundation_UserData                   userData;
};
#else
struct Test_I_Target_Configuration
 : Test_I_CamStream_Configuration
{
  inline Test_I_Target_Configuration ()
   : Test_I_CamStream_Configuration ()
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** socket data **********************************
  Test_I_Target_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** listener data ********************************
  ACE_HANDLE                                      handle;
  //Test_I_Target_IListener_t*               listener;
  struct Test_I_Target_ListenerConfiguration      listenerConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Target_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Test_I_Target_StreamConfiguration_t             streamConfiguration;

  struct Test_I_Target_UserData                   userData;
};
#endif

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Test_I_CamStream_AllocatorConfiguration> Test_I_ControlMessage_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_CamStream_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Target_DirectShow_Stream_Message,
                                          Test_I_Target_DirectShow_Stream_SessionMessage> Test_I_Target_DirectShow_MessageAllocator_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_CamStream_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Target_MediaFoundation_Stream_Message,
                                          Test_I_Target_MediaFoundation_Stream_SessionMessage> Test_I_Target_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_CamStream_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Target_Stream_Message,
                                          Test_I_Target_Stream_SessionMessage> Test_I_Target_MessageAllocator_t;
#endif

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Test_I_Target_DirectShow_ISessionNotify_t> Test_I_Target_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Test_I_Target_MediaFoundation_ISessionNotify_t> Test_I_Target_MediaFoundation_ISubscribe_t;
#else
typedef Common_ISubscribe_T<Test_I_Target_ISessionNotify_t> Test_I_Target_ISubscribe_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_GTK_CBData
 : Test_I_CamStream_GTK_CBData
{
  inline Test_I_Target_DirectShow_GTK_CBData ()
   : Test_I_CamStream_GTK_CBData ()
   , configuration (NULL)
   , progressEventSourceID (0)
   , subscribers ()
  {};

  struct Test_I_Target_DirectShow_Configuration* configuration;
  guint                                          progressEventSourceID;
  Test_I_Target_DirectShow_Subscribers_t         subscribers;
};
struct Test_I_Target_MediaFoundation_GTK_CBData
 : Test_I_CamStream_GTK_CBData
{
  inline Test_I_Target_MediaFoundation_GTK_CBData ()
   : Test_I_CamStream_GTK_CBData ()
   , configuration (NULL)
   , progressEventSourceID (0)
   , subscribers ()
  {};

  struct Test_I_Target_MediaFoundation_Configuration* configuration;
  guint                                               progressEventSourceID;
  Test_I_Target_MediaFoundation_Subscribers_t         subscribers;
};
#else
struct Test_I_Target_GTK_CBData
 : Test_I_CamStream_GTK_CBData
{
  inline Test_I_Target_GTK_CBData ()
   : Test_I_CamStream_GTK_CBData ()
   , configuration (NULL)
   , progressEventSourceID (0)
   , subscribers ()
  {};

  struct Test_I_Target_Configuration* configuration;
  guint                               progressEventSourceID;
  Test_I_Target_Subscribers_t         subscribers;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_UI_GtkBuilderDefinition_T<struct Test_I_Target_DirectShow_GTK_CBData> Test_I_Target_DirectShow_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_I_Target_DirectShow_GTK_CBData> Test_I_Target_DirectShow_GTK_Manager_t;
typedef ACE_Singleton<Test_I_Target_DirectShow_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> TEST_I_TARGET_DIRECTSHOW_GTK_MANAGER_SINGLETON;

typedef Common_UI_GtkBuilderDefinition_T<struct Test_I_Target_MediaFoundation_GTK_CBData> Test_I_Target_MediaFoundation_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_I_Target_MediaFoundation_GTK_CBData> Test_I_Target_MediaFoundation_GTK_Manager_t;
typedef ACE_Singleton<Test_I_Target_MediaFoundation_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> TEST_I_TARGET_MEDIAFOUNDATION_GTK_MANAGER_SINGLETON;
#else
typedef Common_UI_GtkBuilderDefinition_T<struct Test_I_GTK_CBData> Test_I_Target_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_I_GTK_CBData> Test_I_Target_GTK_Manager_t;
typedef ACE_Singleton<Test_I_Target_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> TEST_I_TARGET_GTK_MANAGER_SINGLETON;
#endif

#endif
