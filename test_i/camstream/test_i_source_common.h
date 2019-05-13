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

#include <map>
#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <OAIdl.h>
#include <control.h>
#include <evr.h>
#include <mfapi.h>
#include <strmif.h>
#include <sdkddkver.h>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include <minwindef.h>
#else
#include <windef.h>
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#elif defined (WXWIDGETS_USE)
#include "wx/window.h"
#endif
#endif // GUI_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_statistic_handler.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_messageallocatorheap_base.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_ffmpeg_common.h"
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_i_camstream_common.h"
#include "test_i_camstream_network.h"
#include "test_i_connection_manager_common.h"
#include "test_i_source_eventhandler.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ISampleGrabber;

struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_DirectShow_ConnectionState;
struct Test_I_Source_DirectShow_StreamConfiguration;
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_MediaFoundation_ConnectionState;
struct Test_I_Source_MediaFoundation_StreamConfiguration;
#else
struct Test_I_Source_V4L_ConnectionState;
struct Test_I_Source_V4L_StreamConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
template <typename ConfigurationType>
class Test_I_Source_SignalHandler_T;
template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
class Test_I_Source_EventHandler_T;
extern const char stream_name_string_[];

//struct Test_I_Source_Stream_StatisticData
// : Stream_Statistic
//{
//  Test_I_Source_Stream_StatisticData ()
//   : Stream_Statistic ()
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//   , capturedFrames (0)
//#endif // ACE_WIN32 || ACE_WIN64
//  {}
//
//  struct Test_I_Source_Stream_StatisticData operator+= (const struct Test_I_Source_Stream_StatisticData& rhs_in)
//  {
//    Stream_Statistic::operator+= (rhs_in);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    capturedFrames += rhs_in.capturedFrames;
//#endif // ACE_WIN32 || ACE_WIN64
//
//    return *this;
//  }
//
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  unsigned int capturedFrames;
//#endif // ACE_WIN32 || ACE_WIN64
//};
//typedef Common_IStatistic_T<struct Test_I_Source_Stream_StatisticData> Test_I_Source_Stream_IStatistic_t;
//typedef Common_StatisticHandler_T<struct Test_I_Source_Stream_StatisticData> Test_I_Source_Stream_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Source_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_CamStream_DirectShow_SessionData,
                                        struct _AMMediaType,
                                        struct Test_I_Source_DirectShow_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_Source_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_CamStream_DirectShow_SessionData,
                                   struct _AMMediaType,
                                   struct Test_I_Source_DirectShow_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_Source_DirectShow_SessionData& operator+= (const Test_I_Source_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Stream_SessionDataMediaBase_T<struct Test_I_CamStream_DirectShow_SessionData,
                                  struct _AMMediaType,
                                  struct Test_I_Source_DirectShow_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_Source_DirectShow_SessionData> Test_I_Source_DirectShow_SessionData_t;

class Test_I_Source_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_CamStream_MediaFoundation_SessionData,
                                        IMFMediaType*,
                                        struct Test_I_Source_MediaFoundation_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_Source_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_CamStream_MediaFoundation_SessionData,
                                   IMFMediaType*,
                                   struct Test_I_Source_MediaFoundation_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_Source_MediaFoundation_SessionData& operator+= (const Test_I_Source_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Stream_SessionDataMediaBase_T<struct Test_I_CamStream_MediaFoundation_SessionData,
                                  IMFMediaType*,
                                  struct Test_I_Source_MediaFoundation_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_Source_MediaFoundation_SessionData> Test_I_Source_MediaFoundation_SessionData_t;
#else
class Test_I_Source_V4L_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_CamStream_V4L_SessionData,
                                        struct Stream_MediaFramework_V4L_MediaType,
                                        struct Test_I_Source_V4L_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_Source_V4L_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_CamStream_V4L_SessionData,
                                   struct Stream_MediaFramework_V4L_MediaType,
                                   struct Test_I_Source_V4L_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_Source_V4L_SessionData& operator+= (const Test_I_Source_V4L_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_CamStream_V4L_SessionData,
                                  struct Stream_MediaFramework_V4L_MediaType,
                                  struct Test_I_Source_V4L_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_Source_V4L_SessionData> Test_I_Source_V4L_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Test_I_AllocatorConfiguration> Test_I_ControlMessage_t;

struct Stream_Statistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamState;
struct Test_I_Source_DirectShow_ModuleHandlerConfiguration;
class Test_I_Source_DirectShow_Stream_Message;
class Test_I_Source_DirectShow_SessionMessage;

struct Test_I_Source_MediaFoundation_StreamState;
struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration;
class Test_I_Source_MediaFoundation_Stream_Message;
class Test_I_Source_MediaFoundation_SessionMessage;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_DirectShow_StreamState,
                      struct Test_I_Source_DirectShow_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AllocatorConfiguration,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                      Test_I_Source_DirectShow_SessionData,
                      Test_I_Source_DirectShow_SessionData_t,
                      Test_I_ControlMessage_t,
                      Test_I_Source_DirectShow_Stream_Message,
                      Test_I_Source_DirectShow_SessionMessage> Test_I_Source_DirectShow_StreamBase_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_MediaFoundation_StreamState,
                      struct Test_I_Source_MediaFoundation_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AllocatorConfiguration,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                      Test_I_Source_MediaFoundation_SessionData,
                      Test_I_Source_MediaFoundation_SessionData_t,
                      Test_I_ControlMessage_t,
                      Test_I_Source_MediaFoundation_Stream_Message,
                      Test_I_Source_MediaFoundation_SessionMessage> Test_I_Source_MediaFoundation_StreamBase_t;
#else
struct Test_I_Source_V4L_StreamState;
struct Test_I_Source_V4L_ModuleHandlerConfiguration;
class Test_I_Source_V4L_Stream_Message;
class Test_I_Source_V4L_SessionMessage;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_V4L_StreamState,
                      struct Test_I_Source_V4L_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_AllocatorConfiguration,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                      Test_I_Source_V4L_SessionData,
                      Test_I_Source_V4L_SessionData_t,
                      Test_I_ControlMessage_t,
                      Test_I_Source_V4L_Stream_Message,
                      Test_I_Source_V4L_SessionMessage> Test_I_Source_V4L_StreamBase_t;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Source_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_DirectShow_Stream_Message,
                                    Test_I_Source_DirectShow_SessionMessage> Test_I_Source_DirectShow_ISessionNotify_t;
typedef std::list<Test_I_Source_DirectShow_ISessionNotify_t*> Test_I_Source_DirectShow_Subscribers_t;
typedef Test_I_Source_DirectShow_Subscribers_t::iterator Test_I_Source_DirectShow_SubscribersIterator_t;
struct Test_I_Source_DirectShow_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Source_DirectShow_ModuleHandlerConfiguration ()
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
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   //, sourceFormat ()
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
   , windowController (NULL)
   , windowController2 (NULL)
#endif // GUI_SUPPORT
  {
    finishOnDisconnect = true;

    //mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Test_I_Source_DirectShow_ModuleHandlerConfiguration operator= (const struct Test_I_Source_DirectShow_ModuleHandlerConfiguration& rhs_in)
  {
    area = rhs_in.area;
    if (builder)
    {
      builder->Release (); builder = NULL;
    } // end IF
    if (rhs_in.builder)
    {
      rhs_in.builder->AddRef ();
      builder = rhs_in.builder;
    } // end IF
    if (connection)
    {
      connection->decrease (); connection = NULL;
    } // end IF
    if (rhs_in.connection)
    {
      rhs_in.connection->increase ();
      connection = rhs_in.connection;
    } // end IF
    connectionConfigurations = rhs_in.connectionConfigurations;
    connectionManager = rhs_in.connectionManager;
    filterConfiguration = rhs_in.filterConfiguration;
    filterCLSID = rhs_in.filterCLSID;
    Stream_MediaFramework_DirectShow_Tools::free (outputFormat);
    struct _AMMediaType* media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (rhs_in.outputFormat);
    if (!media_type_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), returning\n")));
      return *this;
    } // end IF
    outputFormat = *media_type_p;
    CoTaskMemFree (media_type_p); media_type_p = NULL;
    push = rhs_in.push;
    //Stream_MediaFramework_DirectShow_Tools::free (sourceFormat);
    //media_type_p =
    //    Stream_MediaFramework_DirectShow_Tools::copy (rhs_in.sourceFormat);
    //if (!media_type_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), returning\n")));
    //  return *this;
    //} // end IF
    //sourceFormat = *media_type_p;
    //CoTaskMemFree (media_type_p); media_type_p = NULL;
    streamConfiguration = rhs_in.streamConfiguration;
    subscriber = rhs_in.subscriber;
    subscribers = rhs_in.subscribers;
    if (windowController)
    {
      windowController->Release (); windowController = NULL;
    } // end IF
    if (rhs_in.windowController)
    {
      rhs_in.windowController->AddRef ();
      windowController = rhs_in.windowController;
    } // end IF
    if (windowController2)
    {
      windowController2->Release (); windowController2 = NULL;
    } // end IF
    if (rhs_in.windowController2)
    {
      rhs_in.windowController2->AddRef ();
      windowController2 = rhs_in.windowController2;
    } // end IF

    return *this;
  }

#if defined (GUI_SUPPORT)
  struct tagRECT                                       area; // visualization module
#endif // GUI_SUPPORT
  IGraphBuilder*                                       builder;
  Test_I_Source_DirectShow_ITCPConnection_t*           connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*                      connectionConfigurations;
  Test_I_Source_DirectShow_TCPConnectionManager_t*     connectionManager; // TCP IO module
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint                                                contextId;
#endif // GTK_USE
#endif // GUI_SUPPORT
  struct Test_I_Source_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                filterCLSID;
  struct _AMMediaType                                  outputFormat; // display module
  bool                                                 push;
  //struct _AMMediaType                                  sourceFormat;
  Test_I_Source_DirectShow_StreamConfiguration_t*      streamConfiguration;
  Test_I_Source_DirectShow_ISessionNotify_t*           subscriber;
  Test_I_Source_DirectShow_Subscribers_t*              subscribers;
#if defined (GUI_SUPPORT)
  IVideoWindow*                                        windowController; // visualization module
  IMFVideoDisplayControl*                              windowController2; // visualization module (EVR)
#endif // GUI_SUPPORT
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Source_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_MediaFoundation_Stream_Message,
                                    Test_I_Source_MediaFoundation_SessionMessage> Test_I_Source_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_I_Source_MediaFoundation_ISessionNotify_t*> Test_I_Source_MediaFoundation_Subscribers_t;
typedef Test_I_Source_MediaFoundation_Subscribers_t::iterator Test_I_Source_MediaFoundation_SubscribersIterator_t;
struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Source_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , mediaSource (NULL)
   , outputFormat (NULL)
   , sampleGrabberNodeId (0)
   , session (NULL)
   //, sourceFormat (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
  {
    finishOnDisconnect = true;

    HRESULT result = MFCreateMediaType (&outputFormat);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    ACE_ASSERT (outputFormat);
  }

  struct tagRECT                                            area;
  Test_I_Source_MediaFoundation_ITCPConnection_t*           connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*                           connectionConfigurations;
  Test_I_Source_MediaFoundation_TCPConnectionManager_t*     connectionManager; // TCP IO module
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx*                                         mediaSource;
#else
  IMFMediaSource*                                           mediaSource;
#endif // _WIN32_WINNT_WIN8
  IMFMediaType*                                             outputFormat; // display module
  TOPOID                                                    sampleGrabberNodeId;
  IMFMediaSession*                                          session;
  //IMFMediaType*                                             sourceFormat;
  Test_I_Source_MediaFoundation_StreamConfiguration_t*      streamConfiguration;
  Test_I_Source_MediaFoundation_ISessionNotify_t*           subscriber;
  Test_I_Source_MediaFoundation_Subscribers_t*              subscribers;
  IMFVideoDisplayControl*                                   windowController;
};
#else
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Source_V4L_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_V4L_Stream_Message,
                                    Test_I_Source_V4L_SessionMessage> Test_I_Source_V4L_ISessionNotify_t;
typedef std::list<Test_I_Source_V4L_ISessionNotify_t*> Test_I_Source_V4L_Subscribers_t;
typedef Test_I_Source_V4L_Subscribers_t::iterator Test_I_Source_V4L_SubscribersIterator_t;
struct Test_I_Source_V4L_StreamConfiguration;
struct Test_I_Source_V4L_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Source_V4L_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
#if defined (GUI_SUPPORT)
   , area ()
#endif // GUI_SUPPORT
   , buffers (STREAM_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , fileDescriptor (-1)
   , method (STREAM_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , outputFormat ()
   //, sourceFormat ()
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
   , window (NULL)
#endif // GUI_SUPPORT
  {
#if defined (GUI_SUPPORT)
    ACE_OS::memset (&area, 0, sizeof (struct v4l2_rect));
#endif // GUI_SUPPORT

    finishOnDisconnect = true;
  }

#if defined (GUI_SUPPORT)
  struct v4l2_rect                              area;
#endif // GUI_SUPPORT
  __u32                                         buffers; // v4l device buffers
  Test_I_Source_V4L_ITCPConnection_t*           connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*               connectionConfigurations;
  Test_I_Source_V4L_TCPConnectionManager_t*     connectionManager; // TCP IO module
  int                                           fileDescriptor;
  enum v4l2_memory                              method; // v4l2 camera source
  struct Stream_MediaFramework_FFMPEG_MediaType outputFormat; // display module
  //struct Stream_MediaFramework_V4L_MediaType     sourceFormat; // source module
  ACE_Time_Value                                statisticCollectionInterval;
  // *TODO*: remove this ASAP
  Test_I_Source_V4L_StreamConfiguration_t*      streamConfiguration;
  Test_I_Source_V4L_ISessionNotify_t*           subscriber;
  Test_I_Source_V4L_Subscribers_t*              subscribers;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkWindow*                                    window;
#elif defined (WXWIDGETS_USE)
  wxWindow*                                     window;
#elif defined (QT_USE)
  XID                                           window;
#endif
#endif // GUI_SUPPORT
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_Source_DirectShow_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
//   , statisticReportingInterval (0)
   , stream (NULL)
  {}

  Test_I_Source_DirectShow_TCPConnectionManager_t*  connectionManager;
//  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_DirectShow_StreamBase_t*            stream;
};
typedef Test_I_Source_SignalHandler_T<struct Test_I_Source_DirectShow_SignalHandlerConfiguration> Test_I_Source_DirectShow_SignalHandler_t;
struct Test_I_Source_MediaFoundation_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_Source_MediaFoundation_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   //   , statisticReportingInterval (0)
   , stream (NULL)
  {}

  Test_I_Source_MediaFoundation_TCPConnectionManager_t*  connectionManager;
  //  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_MediaFoundation_StreamBase_t*            stream;
};
typedef Test_I_Source_SignalHandler_T<struct Test_I_Source_MediaFoundation_SignalHandlerConfiguration> Test_I_Source_MediaFoundation_SignalHandler_t;
#else
struct Test_I_Source_V4L_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_Source_V4L_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   //   , statisticReportingInterval (0)
   , stream (NULL)
  {}

  Test_I_Source_V4L_TCPConnectionManager_t* connectionManager;
  Test_I_Source_V4L_StreamBase_t*           stream;
};
typedef Test_I_Source_SignalHandler_T<struct Test_I_Source_V4L_SignalHandlerConfiguration> Test_I_Source_V4L_SignalHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Source_DirectShow_StreamConfiguration ()
   : Stream_Configuration ()
   , format ()
   , graphLayout ()
  {
    ACE_OS::memset (&format, 0, sizeof (struct _AMMediaType));
  }

  // **************************** stream data **********************************
  struct _AMMediaType                       format;
  Stream_MediaFramework_DirectShow_Graph_t  graphLayout;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AllocatorConfiguration,
                               struct Test_I_Source_DirectShow_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_Source_DirectShow_ModuleHandlerConfiguration> Test_I_Source_DirectShow_StreamConfiguration_t;
typedef Test_I_Source_DirectShow_StreamConfiguration_t::ITERATOR_T Test_I_Source_DirectShow_StreamConfigurationIterator_t;
typedef std::map<std::string,
                 Test_I_Source_DirectShow_StreamConfiguration_t> Test_I_Source_DirectShow_StreamConfigurations_t;
typedef Test_I_Source_DirectShow_StreamConfigurations_t::iterator Test_I_Source_DirectShow_StreamConfigurationsIterator_t;

struct Test_I_MediaFoundationConfiguration;
struct Test_I_Source_MediaFoundation_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Source_MediaFoundation_StreamConfiguration ()
   : Stream_Configuration ()
   , format (NULL)
   , mediaFoundationConfiguration (NULL)
  {}

  // **************************** stream data **********************************
  IMFMediaType*                               format;

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration* mediaFoundationConfiguration;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AllocatorConfiguration,
                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration> Test_I_Source_MediaFoundation_StreamConfiguration_t;
typedef Test_I_Source_MediaFoundation_StreamConfiguration_t::ITERATOR_T Test_I_Source_MediaFoundation_StreamConfigurationIterator_t;
typedef std::map<std::string,
                 Test_I_Source_MediaFoundation_StreamConfiguration_t> Test_I_Source_MediaFoundation_StreamConfigurations_t;
typedef Test_I_Source_MediaFoundation_StreamConfigurations_t::iterator Test_I_Source_MediaFoundation_StreamConfigurationsIterator_t;
#else
struct Test_I_Source_V4L_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Source_V4L_StreamConfiguration ()
   : Stream_Configuration ()
   , format ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_V4L_MediaType format;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AllocatorConfiguration,
                               struct Test_I_Source_V4L_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_Source_V4L_ModuleHandlerConfiguration> Test_I_Source_V4L_StreamConfiguration_t;
typedef Test_I_Source_V4L_StreamConfiguration_t::ITERATOR_T Test_I_Source_V4L_StreamConfigurationIterator_t;
typedef std::map<std::string,
                 Test_I_Source_V4L_StreamConfiguration_t> Test_I_Source_V4L_StreamConfigurations_t;
typedef Test_I_Source_V4L_StreamConfigurations_t::iterator Test_I_Source_V4L_StreamConfigurationsIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamState
 : Stream_State
{
  Test_I_Source_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_Source_DirectShow_SessionData* sessionData;
};

struct Test_I_Source_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_Source_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_Source_MediaFoundation_SessionData* sessionData;
};
#else
struct Test_I_Source_V4L_StreamState
 : Stream_State
{
  Test_I_Source_V4L_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_Source_V4L_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_Configuration
 : Test_I_CamStream_Configuration
{
  Test_I_Source_DirectShow_Configuration ()
   : Test_I_CamStream_Configuration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfigurations ()
  {}

  // **************************** signal data **********************************
  struct Test_I_Source_DirectShow_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                             connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_Source_DirectShow_StreamConfigurations_t            streamConfigurations;
};

struct Test_I_Source_MediaFoundation_Configuration
 : Test_I_CamStream_Configuration
{
  Test_I_Source_MediaFoundation_Configuration ()
   : Test_I_CamStream_Configuration ()
   , mediaFoundationConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfigurations ()
  {}

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration                      mediaFoundationConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Source_MediaFoundation_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                                  connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_Source_MediaFoundation_StreamConfigurations_t            streamConfigurations;
};
#else
struct Test_I_Source_V4L_Configuration
 : Test_I_CamStream_Configuration
{
  Test_I_Source_V4L_Configuration ()
   : Test_I_CamStream_Configuration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfigurations ()
  {}

  // **************************** signal data **********************************
  struct Test_I_Source_V4L_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                      connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_Source_V4L_StreamConfigurations_t            streamConfigurations;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_ControlMessage_T<ENUM Stream_ControlMessageType,
//                                struct Test_I_AllocatorConfiguration> Test_I_DirectShow_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage> Test_I_Source_DirectShow_MessageAllocator_t;

struct Test_I_Source_DirectShow_UI_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     Test_I_Source_DirectShow_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_DirectShow_Stream_Message,
                                     Test_I_Source_DirectShow_SessionMessage,
                                     struct Test_I_Source_DirectShow_UI_CBData> Test_I_Source_DirectShow_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_Source_DirectShow_ISessionNotify_t> Test_I_Source_DirectShow_ISubscribe_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_MediaFoundation_Stream_Message,
                                          Test_I_Source_MediaFoundation_SessionMessage> Test_I_Source_MediaFoundation_MessageAllocator_t;

struct Test_I_Source_MediaFoundation_UI_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     Test_I_Source_MediaFoundation_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_MediaFoundation_Stream_Message,
                                     Test_I_Source_MediaFoundation_SessionMessage,
                                     struct Test_I_Source_MediaFoundation_UI_CBData> Test_I_Source_MediaFoundation_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_Source_MediaFoundation_ISessionNotify_t> Test_I_Source_MediaFoundation_ISubscribe_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_V4L_Stream_Message,
                                          Test_I_Source_V4L_SessionMessage> Test_I_Source_V4L_MessageAllocator_t;

struct Test_I_Source_V4L_UI_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     Test_I_Source_V4L_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_V4L_Stream_Message,
                                     Test_I_Source_V4L_SessionMessage,
                                     struct Test_I_Source_V4L_UI_CBData> Test_I_Source_V4L_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_Source_V4L_ISessionNotify_t> Test_I_Source_V4L_ISubscribe_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_UI_CBData
 : Test_I_CamStream_UI_CBData
{
  Test_I_Source_DirectShow_UI_CBData ()
   : Test_I_CamStream_UI_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
   , streamConfiguration (NULL)
   , UDPStream (NULL)
  {}

  struct Test_I_Source_DirectShow_Configuration* configuration;
  Test_I_Source_DirectShow_StreamBase_t*         stream;
  Test_I_Source_DirectShow_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                      subscribersLock;
  IAMStreamConfig*                               streamConfiguration;
  Test_I_Source_DirectShow_StreamBase_t*         UDPStream;
};

struct Test_I_Source_MediaFoundation_UI_CBData
 : Test_I_CamStream_UI_CBData
{
  Test_I_Source_MediaFoundation_UI_CBData ()
   : Test_I_CamStream_UI_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
   , UDPStream (NULL)
  {}

  struct Test_I_Source_MediaFoundation_Configuration* configuration;
  Test_I_Source_MediaFoundation_StreamBase_t*         stream;
  Test_I_Source_MediaFoundation_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                           subscribersLock;
  Test_I_Source_MediaFoundation_StreamBase_t*         UDPStream;
};
#else
struct Test_I_Source_V4L_UI_CBData
 : Test_I_CamStream_UI_CBData
{
  Test_I_Source_V4L_UI_CBData ()
   : Test_I_CamStream_UI_CBData ()
   , configuration (NULL)
   , fileDescriptor (-1)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
   , UDPStream (NULL)
  {}

  struct Test_I_Source_V4L_Configuration* configuration;
  int                                     fileDescriptor; // (capture) device file descriptor
  Test_I_Source_V4L_StreamBase_t*         stream;
  Test_I_Source_V4L_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX               subscribersLock;
  Test_I_Source_V4L_StreamBase_t*         UDPStream;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_ThreadData
 : Test_I_CamStream_ThreadData
{
  Test_I_Source_DirectShow_ThreadData ()
   : Test_I_CamStream_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_Source_DirectShow_UI_CBData* CBData;
};

struct Test_I_Source_MediaFoundation_ThreadData
 : Test_I_CamStream_ThreadData
{
  Test_I_Source_MediaFoundation_ThreadData ()
   : Test_I_CamStream_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_Source_MediaFoundation_UI_CBData* CBData;
};
#else
struct Test_I_Source_V4L_ThreadData
 : Test_I_CamStream_ThreadData
{
  Test_I_Source_V4L_ThreadData ()
   : Test_I_CamStream_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_Source_V4L_UI_CBData* CBData;
};
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#endif
