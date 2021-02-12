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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <list>
#endif // ACE_WIN32 || ACE_WIN64
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <BaseTyps.h>
#include <OAIdl.h>
#include <control.h>
#include <d3d9.h>
#include <evr.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <strmif.h>
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/INET_Addr.h"
#include "ace/os_include/sys/os_socket.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_control_message.h"
#include "stream_isessionnotify.h"

#include "stream_dec_defines.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#include "stream_lib_ffmpeg_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_guids.h"
#include "stream_lib_mediafoundation_mediasource.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "net_defines.h"
#include "net_ilistener.h"

#include "test_i_defines.h"

#include "test_i_camstream_common.h"
#include "test_i_camstream_network.h"
#include "test_i_connection_manager_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Target_DirectShow_Stream_Message;
class Test_I_Target_DirectShow_SessionMessage;
class Test_I_Target_MediaFoundation_Stream_Message;
class Test_I_Target_MediaFoundation_SessionMessage;
#else
struct v4l2_window;
class Test_I_Target_Stream_Message;
class Test_I_Target_SessionMessage;
#endif // ACE_WIN32 || ACE_WIN64
template <typename ConfigurationType,
          typename TCPConnectionManagerType,
          typename UDPConnectionManagerType>
class Test_I_Target_SignalHandler_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_MessageData
{
  Test_I_Target_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0.0)
  {}

  IMediaSample* sample;
  double        sampleTime;
};
struct Test_I_Target_MediaFoundation_MessageData
{
  Test_I_Target_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMFMediaBuffer* sample;
  LONGLONG        sampleTime;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//struct Test_I_Target_DirectShow_ConnectionConfiguration;
//struct Test_I_Target_DirectShow_StreamConfiguration;
//struct Test_I_Target_MediaFoundation_ConnectionConfiguration;
//struct Test_I_Target_MediaFoundation_StreamConfiguration;
//struct Test_I_Target_MediaFoundation_UserData
// : Stream_UserData
//{
//  Test_I_Target_MediaFoundation_UserData ()
//   : Stream_UserData ()
//   //, connectionConfiguration (NULL)
//   //, streamConfiguration (NULL)
//  {}
//
//  //struct Test_I_Target_MediaFoundation_ConnectionConfiguration* connectionConfiguration;
//  //struct Test_I_Target_MediaFoundation_StreamConfiguration*     streamConfiguration;
//};
#else
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Target_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_CamStream_DirectShow_SessionData,
                                        struct _AMMediaType,
                                        struct Test_I_Target_DirectShow_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_Target_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_CamStream_DirectShow_SessionData,
                                   struct _AMMediaType,
                                   struct Test_I_Target_DirectShow_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
   , targetFileName ()
   , windowController (NULL)
  {}

  Test_I_Target_DirectShow_SessionData& operator+= (const Test_I_Target_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_CamStream_DirectShow_SessionData,
                                  struct _AMMediaType,
                                  struct Test_I_Target_DirectShow_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);

    return *this;
  }

  Net_IINETConnection_t* connection;
  std::string            targetFileName;
  IVideoWindow*          windowController;
};
typedef Stream_SessionData_T<Test_I_Target_DirectShow_SessionData> Test_I_Target_DirectShow_SessionData_t;

class Test_I_Target_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_CamStream_MediaFoundation_SessionData,
                                        IMFMediaType*,
                                        struct Test_I_Target_MediaFoundation_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_Target_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_CamStream_MediaFoundation_SessionData,
                                   IMFMediaType*,
                                   struct Test_I_Target_MediaFoundation_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
   , outputFormat (NULL)
   , sourceFormat (NULL)
  {}

  Test_I_Target_MediaFoundation_SessionData& operator+= (const Test_I_Target_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_CamStream_MediaFoundation_SessionData,
                                  IMFMediaType*,
                                  struct Test_I_Target_MediaFoundation_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);

    return *this;
  }

  Net_IINETConnection_t* connection;
  IMFMediaType*          outputFormat;
  IMFMediaType*          sourceFormat;
};
typedef Stream_SessionData_T<Test_I_Target_MediaFoundation_SessionData> Test_I_Target_MediaFoundation_SessionData_t;
#else
class Test_I_Target_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_CamStream_V4L_SessionData,
                                        struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                                        struct Test_I_Target_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_Target_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_CamStream_V4L_SessionData,
                                   struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                                   struct Test_I_Target_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , connection (NULL)
  {}

  Test_I_Target_SessionData& operator+= (const Test_I_Target_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_CamStream_V4L_SessionData,
                                  struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                                  struct Test_I_Target_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);
    targetFileName =
      (targetFileName.empty () ? rhs_in.targetFileName : targetFileName);

    return *this;
  }

  Net_IINETConnection_t* connection;
};
typedef Stream_SessionData_T<Test_I_Target_SessionData> Test_I_Target_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Test_I_Target_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   //, format (NULL)
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  // *TODO*: specify this as part of the network protocol header/handshake
  //struct _AMMediaType*                                           format; // handle
  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Target_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Target_DirectShow_Stream_Message,
                                    Test_I_Target_DirectShow_SessionMessage> Test_I_Target_DirectShow_ISessionNotify_t;
typedef std::list<Test_I_Target_DirectShow_ISessionNotify_t*> Test_I_Target_DirectShow_Subscribers_t;
typedef Test_I_Target_DirectShow_Subscribers_t::iterator Test_I_Target_DirectShow_SubscribersIterator_t;
struct Test_I_Target_DirectShow_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Target_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
#if defined (GUI_SUPPORT)
   , area ()
#endif // GUI_SUPPORT
   , builder (NULL)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
#endif // GUI_SUPPORT
   , crunch (true)
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , queue (NULL)
   , sourceFormat ()
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
   , window (NULL)
   , windowController (NULL)
   , windowController2 (NULL)
#endif // GUI_SUPPORT
  {
    inbound = true;

    push = true; // *TODO*: support asynch directshow filter
    filterCLSID =
      (push ? CLSID_ACEStream_MediaFramework_Source_Filter
            : CLSID_ACEStream_MediaFramework_Asynch_Source_Filter);
  }

#if defined (GUI_SUPPORT)
  struct tagRECT                                       area;              // display module
#endif // GUI_SUPPORT
  IGraphBuilder*                                       builder;           // display module
  Net_IINETConnection_t*                               connection;        // Net source/IO module
  Net_ConnectionConfigurations_t*                      connectionConfigurations;
  Test_I_Target_DirectShow_TCPConnectionManager_t*     connectionManager; // Net IO module
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint                                                contextId; // status bar-
#endif // GTK_USE
#endif // GUI_SUPPORT
  bool                                                 crunch;            // splitter module
  struct Test_I_Target_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                filterCLSID;
  struct _AMMediaType                                  outputFormat; // display module
  bool                                                 push; // media sample passing strategy
  ACE_Message_Queue_Base*                              queue; // (inbound) buffer queue handle
  struct _AMMediaType                                  sourceFormat;
  Test_I_Target_DirectShow_StreamConfiguration_t*      streamConfiguration;
  Test_I_Target_DirectShow_ISessionNotify_t*           subscriber;        // event handler module
  Test_I_Target_DirectShow_Subscribers_t*              subscribers;       // event handler module
#if defined (GUI_SUPPORT)
  HWND                                                 window;            // display module
  IVideoWindow*                                        windowController;  // display module
  IMFVideoDisplayControl*                              windowController2; // display module: EVR
#endif // GUI_SUPPORT
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Target_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Target_MediaFoundation_Stream_Message,
                                    Test_I_Target_MediaFoundation_SessionMessage> Test_I_Target_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_I_Target_MediaFoundation_ISessionNotify_t*> Test_I_Target_MediaFoundation_Subscribers_t;
typedef Test_I_Target_MediaFoundation_Subscribers_t::iterator Test_I_Target_MediaFoundation_SubscribersIterator_t;
struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Target_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
#if defined (GUI_SUPPORT)
   , area ()
#endif // GUI_SUPPORT
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
#endif // GUI_SUPPORT
   , crunch (true)
   , deviceIdentifier ()
#if defined (GUI_SUPPORT)
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
#endif // GUI_SUPPORT
   , mediaSource (NULL)
   , outputFormat (NULL)
   , queue (NULL)
   //, sourceReader (NULL)
   , rendererNodeId (0)
   , session (NULL)
   , sourceFormat (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
   , window (NULL)
   , windowController (NULL)
#endif // GUI_SUPPORT
  {
    inbound = true;

    HRESULT result = MFCreateMediaType (&outputFormat);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  }

#if defined (GUI_SUPPORT)
  struct tagRECT                                        area;                      // display module
#endif // GUI_SUPPORT
  Net_IINETConnection_t*                                connection;                // net source/IO module
  Net_ConnectionConfigurations_t*                       connectionConfigurations;
  Test_I_Target_MediaFoundation_TCPConnectionManager_t* connectionManager;         // net IO module
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint                                                 contextId; // status bar-
#endif // GTK_USE
#endif // GUI_SUPPORT
  bool                                                  crunch;                    // splitter module
  std::string                                           deviceIdentifier;
  IDirect3DDevice9Ex*                                   direct3DDevice;            // display module
  UINT                                                  direct3DManagerResetToken; // display module
  IMFMediaType*                                         outputFormat;               // display module
  IMFMediaSource*                                       mediaSource;
  ACE_Message_Queue_Base*                               queue; // (inbound) buffer queue handle
  TOPOID                                                rendererNodeId;            // display module
  //IMFSourceReaderEx*                                             sourceReader;
  IMFMediaSession*                                      session;
  IMFMediaType*                                         sourceFormat;               // source module
  Test_I_Target_MediaFoundation_StreamConfiguration_t*  streamConfiguration;
  Test_I_Target_MediaFoundation_ISessionNotify_t*       subscriber;                // event handler module
  Test_I_Target_MediaFoundation_Subscribers_t*          subscribers;               // event handler module
#if defined (GUI_SUPPORT)
  HWND                                                  window;                    // display module
  IMFVideoDisplayControl*                               windowController;          // display module
#endif // GUI_SUPPORT
};
#else
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Target_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Target_Stream_Message,
                                    Test_I_Target_SessionMessage> Test_I_Target_ISessionNotify_t;
typedef std::list<Test_I_Target_ISessionNotify_t*> Test_I_Target_Subscribers_t;
typedef Test_I_Target_Subscribers_t::iterator Test_I_Target_SubscribersIterator_t;
struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Target_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , area ()
#endif // GTK_USE
#endif // GUI_SUPPORT
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
#endif // GUI_SUPPORT
   , crunch (false)
   , outputFormat ()
   , queue (NULL)
   , streamConfiguration (NULL)
   , targetFileName ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
    inbound = true;
  }

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkRectangle                                  area;
#endif // GTK_USE
#endif // GUI_SUPPORT
  Net_ConnectionConfigurations_t*               connectionConfigurations;
  Test_I_Target_TCPConnectionManager_t*         connectionManager; // net IO module
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint                                         contextId;
#endif // GTK_USE
#endif // GUI_SUPPORT
  bool                                          crunch;            // splitter module
  struct Stream_MediaFramework_FFMPEG_VideoMediaType outputFormat; // gtk pixbuf module
  ACE_Message_Queue_Base*                       queue;  // (inbound) buffer queue handle
  // *TODO*: remove this ASAP
  Test_I_Target_StreamConfiguration_t*          streamConfiguration;
  std::string                                   targetFileName;    // file writer module
  Test_I_Target_ISessionNotify_t*               subscriber;
  Test_I_Target_Subscribers_t*                  subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//class Test_I_Target_DirectShow_ListenerConfiguration
// : public Net_ListenerConfiguration_T<Test_I_Target_DirectShow_TCPConnectionConfiguration_t,
//                                      NET_TRANSPORTLAYER_TCP>
//{
// public:
//  Test_I_Target_DirectShow_ListenerConfiguration ()
//   : Net_ListenerConfiguration_T ()
//   , connectionConfiguration (NULL)
//   , connectionManager (NULL)
//   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
//  {}
//
//  Test_I_Target_DirectShow_TCPConnectionConfiguration_t* connectionConfiguration;
//  Test_I_Target_DirectShow_TCPConnectionManager_t*       connectionManager;
//  ACE_Time_Value                                         statisticReportingInterval; // [ACE_Time_Value::zero: off]
//};
typedef Net_IListener_T<Test_I_Target_DirectShow_TCPConnectionConfiguration_t> Test_I_Target_DirectShow_IListener_t;

//class Test_I_Target_MediaFoundation_ListenerConfiguration
// : public Net_ListenerConfiguration_T<Test_I_Target_MediaFoundation_TCPConnectionConfiguration_t,
//                                      NET_TRANSPORTLAYER_TCP>
//{
// public:
//  Test_I_Target_MediaFoundation_ListenerConfiguration ()
//   : Net_ListenerConfiguration_T ()
//   , connectionConfiguration (NULL)
//   , connectionManager (NULL)
//   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
//  {}
//
//  Test_I_Target_MediaFoundation_TCPConnectionConfiguration_t* connectionConfiguration;
//  Test_I_Target_MediaFoundation_TCPConnectionManager_t*       connectionManager;
//  ACE_Time_Value                                              statisticReportingInterval; // [ACE_Time_Value::zero: off]
//};
typedef Net_IListener_T<Test_I_Target_MediaFoundation_TCPConnectionConfiguration_t> Test_I_Target_MediaFoundation_IListener_t;
#else
//class Test_I_Target_ListenerConfiguration
// : public Net_ListenerConfiguration_T<Test_I_Target_TCPConnectionConfiguration_t,
//                                      NET_TRANSPORTLAYER_TCP>
//{
// public:
//  Test_I_Target_ListenerConfiguration ()
//   : Net_ListenerConfiguration_T ()
//   , connectionConfiguration (NULL)
//   , connectionManager (NULL)
//   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
//  {
//    //address.set_port_number (TEST_I_DEFAULT_PORT, 1);
//  }
//
//  Test_I_Target_TCPConnectionConfiguration_t* connectionConfiguration;
//  Test_I_Target_TCPConnectionManager_t*       connectionManager;
//  ACE_Time_Value                              statisticReportingInterval; // [ACE_Time_Value::zero: off]
//};
typedef Net_IListener_T<Test_I_Target_TCPConnectionConfiguration_t> Test_I_Target_IListener_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_Target_DirectShow_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   , listener (NULL)
   , statisticReportingHandler (NULL)
  {}

  Net_IConnectionManagerBase_t*         connectionManager;
  Test_I_Target_DirectShow_IListener_t* listener;
  Net_IStreamStatisticHandler_t*        statisticReportingHandler;
};
typedef Test_I_Target_SignalHandler_T<struct Test_I_Target_DirectShow_SignalHandlerConfiguration,
                                      Test_I_Target_DirectShow_TCPConnectionManager_t,
                                      Test_I_Target_DirectShow_UDPConnectionManager_t> Test_I_Target_DirectShow_SignalHandler_t;
struct Test_I_Target_MediaFoundation_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_Target_MediaFoundation_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   , listener (NULL)
   , statisticReportingHandler (NULL)
  {}

  Net_IConnectionManagerBase_t*              connectionManager;
  Test_I_Target_MediaFoundation_IListener_t* listener;
  Net_IStreamStatisticHandler_t*             statisticReportingHandler;
};
typedef Test_I_Target_SignalHandler_T<struct Test_I_Target_MediaFoundation_SignalHandlerConfiguration,
                                      Test_I_Target_MediaFoundation_TCPConnectionManager_t,
                                      Test_I_Target_MediaFoundation_UDPConnectionManager_t> Test_I_Target_MediaFoundation_SignalHandler_t;
#else
struct Test_I_Target_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_Target_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   , listener (NULL)
   , statisticReportingHandler (NULL)
  {}

  Test_I_Target_TCPConnectionManager_t* connectionManager;
  Test_I_Target_IListener_t*            listener;
  Net_IStreamStatisticHandler_t*        statisticReportingHandler;
};
typedef Test_I_Target_SignalHandler_T<struct Test_I_Target_SignalHandlerConfiguration,
                                      Test_I_Target_TCPConnectionManager_t,
                                      Test_I_Target_UDPConnectionManager_t> Test_I_Target_SignalHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Target_DirectShow_StreamConfiguration ()
   : Stream_Configuration ()
   , graphBuilder (NULL)
  {}

  IGraphBuilder* graphBuilder;
};
struct Test_I_Target_MediaFoundation_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Target_MediaFoundation_StreamConfiguration ()
   : Stream_Configuration ()
  {}
};
#else
struct Test_I_Target_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Target_StreamConfiguration ()
   : Stream_Configuration ()
   , format ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , window (NULL)
#endif // GTK_USE
#endif // GUI_SUPPORT
  {}

  struct Stream_MediaFramework_FFMPEG_VideoMediaType format;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkWindow*                                    window;
#endif // GTK_USE
#endif // GUI_SUPPORT
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_StreamState
 : Stream_State
{
  Test_I_Target_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_Target_DirectShow_SessionData* sessionData;
};

struct Test_I_Target_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_Target_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_Target_MediaFoundation_SessionData* sessionData;
};
#else
struct Test_I_Target_StreamState
 : Stream_State
{
  Test_I_Target_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_Target_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_Configuration
 : Test_I_CamStream_Configuration
{
  Test_I_Target_DirectShow_Configuration ()
   : Test_I_CamStream_Configuration ()
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   //, listenerConfiguration ()
   , signalHandlerConfiguration ()
   , pinConfiguration ()
   , filterConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                                 connectionConfigurations;
  // **************************** listener data ********************************
  ACE_HANDLE                                                     handle;
  //Test_I_Target_IListener_t*               listener;
  //Test_I_Target_DirectShow_ListenerConfiguration                 listenerConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Target_DirectShow_SignalHandlerConfiguration     signalHandlerConfiguration;
  // **************************** stream data **********************************
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration pinConfiguration;
  struct Test_I_Target_DirectShow_FilterConfiguration            filterConfiguration;
  Test_I_Target_DirectShow_StreamConfiguration_t                 streamConfiguration;
};
struct Test_I_Target_MediaFoundation_Configuration
 : Test_I_CamStream_Configuration
{
  Test_I_Target_MediaFoundation_Configuration ()
   : Test_I_CamStream_Configuration ()
   , mediaFoundationConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   //, listenerConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration                      mediaFoundationConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Target_MediaFoundation_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                                  connectionConfigurations;
  // **************************** listener data ********************************
  ACE_HANDLE                                                      handle;
  //Test_I_Target_IListener_t*                             listener;
  //Test_I_Target_MediaFoundation_ListenerConfiguration             listenerConfiguration;
  // **************************** stream data **********************************
  Test_I_Target_MediaFoundation_StreamConfiguration_t             streamConfiguration;
};
#else
struct Test_I_Target_Configuration
 : Test_I_CamStream_Configuration
{
  Test_I_Target_Configuration ()
   : Test_I_CamStream_Configuration ()
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
//   , listenerConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                  connectionConfigurations;
  // **************************** listener data ********************************
  ACE_HANDLE                                      handle;
  //Test_I_Target_IListener_t*               listener;
//  struct Test_I_Target_ListenerConfiguration      listenerConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Target_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Test_I_Target_StreamConfiguration_t             streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Target_DirectShow_Stream_Message,
                                          Test_I_Target_DirectShow_SessionMessage> Test_I_Target_DirectShow_MessageAllocator_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Target_MediaFoundation_Stream_Message,
                                          Test_I_Target_MediaFoundation_SessionMessage> Test_I_Target_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Target_Stream_Message,
                                          Test_I_Target_SessionMessage> Test_I_Target_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Test_I_Target_DirectShow_ISessionNotify_t> Test_I_Target_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Test_I_Target_MediaFoundation_ISessionNotify_t> Test_I_Target_MediaFoundation_ISubscribe_t;
#else
typedef Common_ISubscribe_T<Test_I_Target_ISessionNotify_t> Test_I_Target_ISubscribe_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_UI_CBData
 : Test_I_CamStream_UI_CBData
{
  Test_I_Target_DirectShow_UI_CBData ()
   : Test_I_CamStream_UI_CBData ()
   , configuration (NULL)
   , subscribers ()
  {}

  struct Test_I_Target_DirectShow_Configuration* configuration;
  Test_I_Target_DirectShow_Subscribers_t         subscribers;
};

struct Test_I_Target_MediaFoundation_UI_CBData
 : Test_I_CamStream_UI_CBData
{
  Test_I_Target_MediaFoundation_UI_CBData ()
   : Test_I_CamStream_UI_CBData ()
   , configuration (NULL)
   , subscribers ()
  {}

  struct Test_I_Target_MediaFoundation_Configuration* configuration;
  Test_I_Target_MediaFoundation_Subscribers_t         subscribers;
};
#else
struct Test_I_Target_UI_CBData
 : Test_I_CamStream_UI_CBData
{
  Test_I_Target_UI_CBData ()
   : Test_I_CamStream_UI_CBData ()
   , configuration (NULL)
   , subscribers ()
  {}

  struct Test_I_Target_Configuration* configuration;
  Test_I_Target_Subscribers_t         subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MediaFramework_MediaFoundation_MediaSource_T<Common_TimePolicy_t,
                                                            Test_I_Target_MediaFoundation_SessionMessage,
                                                            Test_I_Target_MediaFoundation_Stream_Message,
                                                            struct Test_I_MediaFoundationConfiguration,
                                                            IMFMediaType*> Stream_MediaFramework_MediaFoundation_MediaSource_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
