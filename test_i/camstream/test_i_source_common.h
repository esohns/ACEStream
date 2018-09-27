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
#else
//#include <linux/videodev2.h>
#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_statistic_handler.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

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
struct ISampleGrabber;

struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_DirectShow_ConnectionState;
struct Test_I_Source_DirectShow_StreamConfiguration;
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_MediaFoundation_ConnectionState;
struct Test_I_Source_MediaFoundation_StreamConfiguration;
#else
struct Test_I_Source_V4L2_ConnectionConfiguration;
struct Test_I_Source_V4L2_ConnectionState;
struct Test_I_Source_V4L2_StreamConfiguration;
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_UserData
 : Stream_UserData
{
  Test_I_Source_DirectShow_UserData ()
   : Stream_UserData ()
   , connectionConfiguration (NULL)
   , streamConfiguration (NULL)
  {}

  struct Test_I_Source_DirectShow_ConnectionConfiguration* connectionConfiguration;
  struct Test_I_Source_DirectShow_StreamConfiguration*     streamConfiguration;
};
struct Test_I_Source_MediaFoundation_UserData
 : Stream_UserData
{
  Test_I_Source_MediaFoundation_UserData ()
   : Stream_UserData ()
   , connectionConfiguration (NULL)
   , streamConfiguration (NULL)
  {}

  struct Test_I_Source_MediaFoundation_ConnectionConfiguration* connectionConfiguration;
  struct Test_I_Source_MediaFoundation_StreamConfiguration*     streamConfiguration;
};
#else
struct Test_I_Source_V4L2_ConnectionConfiguration;
struct Test_I_Source_V4L2_UserData
 : Stream_UserData
{
  Test_I_Source_V4L2_UserData ()
   : Stream_UserData ()
//   , connectionConfiguration (NULL)
//   , streamConfiguration (NULL)
  {}

//  struct Test_I_Source_V4L2_ConnectionConfiguration* connectionConfiguration;
//  struct Test_I_Source_V4L2_StreamConfiguration*     streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_SessionData
 : Test_I_CamStream_DirectShow_SessionData
{
  Test_I_Source_DirectShow_SessionData ()
   : Test_I_CamStream_DirectShow_SessionData ()
   , userData (NULL)
  {}

  struct Test_I_Source_DirectShow_SessionData& operator+= (const struct Test_I_Source_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_CamStream_DirectShow_SessionData::operator+= (rhs_in);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Source_DirectShow_UserData* userData;
};
typedef Stream_SessionData_T<struct Test_I_Source_DirectShow_SessionData> Test_I_Source_DirectShow_SessionData_t;
struct Test_I_Source_MediaFoundation_SessionData
 : Test_I_CamStream_MediaFoundation_SessionData
{
  Test_I_Source_MediaFoundation_SessionData ()
   : Test_I_CamStream_MediaFoundation_SessionData ()
   , userData (NULL)
  {}

  struct Test_I_Source_MediaFoundation_SessionData& operator+= (const struct Test_I_Source_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_CamStream_MediaFoundation_SessionData::operator+= (rhs_in);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_Source_MediaFoundation_UserData* userData;
};
typedef Stream_SessionData_T<struct Test_I_Source_MediaFoundation_SessionData> Test_I_Source_MediaFoundation_SessionData_t;
#else
struct Test_I_Source_V4L2_SessionData
 : Test_I_CamStream_V4L2_SessionData
{
  Test_I_Source_V4L2_SessionData ()
   : Test_I_CamStream_V4L2_SessionData ()
   , height (0)
   , width (0)
   , userData (NULL)
  {}

  struct Test_I_Source_V4L2_SessionData& operator+= (const struct Test_I_Source_V4L2_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_CamStream_V4L2_SessionData::operator+= (rhs_in);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  unsigned int height;
  unsigned int width;

  struct Test_I_Source_V4L2_UserData* userData;
};
typedef Stream_SessionData_T<struct Test_I_Source_V4L2_SessionData> Test_I_Source_V4L2_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Test_I_AllocatorConfiguration> Test_I_ControlMessage_t;

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
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_DirectShow_StreamState,
                      struct Test_I_Source_DirectShow_StreamConfiguration,
                      struct Test_I_Source_Stream_StatisticData,
                      struct Test_I_AllocatorConfiguration,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                      struct Test_I_Source_DirectShow_SessionData,
                      Test_I_Source_DirectShow_SessionData_t,
                      Test_I_ControlMessage_t,
                      Test_I_Source_DirectShow_Stream_Message,
                      Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_Source_DirectShow_StreamBase_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_MediaFoundation_StreamState,
                      struct Test_I_Source_MediaFoundation_StreamConfiguration,
                      struct Test_I_Source_Stream_StatisticData,
                      struct Test_I_AllocatorConfiguration,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                      struct Test_I_Source_MediaFoundation_SessionData,
                      Test_I_Source_MediaFoundation_SessionData_t,
                      Test_I_ControlMessage_t,
                      Test_I_Source_MediaFoundation_Stream_Message,
                      Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_Source_MediaFoundation_StreamBase_t;
#else
struct Test_I_Source_V4L2_StreamState;
struct Test_I_Source_V4L2_ModuleHandlerConfiguration;
class Test_I_Source_V4L2_Stream_Message;
class Test_I_Source_V4L2_Stream_SessionMessage;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_V4L2_StreamState,
                      struct Test_I_Source_V4L2_StreamConfiguration,
                      struct Test_I_Source_Stream_StatisticData,
                      struct Test_I_AllocatorConfiguration,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                      struct Test_I_Source_V4L2_SessionData,
                      Test_I_Source_V4L2_SessionData_t,
                      Test_I_ControlMessage_t,
                      Test_I_Source_V4L2_Stream_Message,
                      Test_I_Source_V4L2_Stream_SessionMessage> Test_I_Source_V4L2_StreamBase_t;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_DirectShow_Stream_Message,
                                    Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_Source_DirectShow_ISessionNotify_t;
typedef std::list<Test_I_Source_DirectShow_ISessionNotify_t*> Test_I_Source_DirectShow_Subscribers_t;
typedef Test_I_Source_DirectShow_Subscribers_t::iterator Test_I_Source_DirectShow_SubscribersIterator_t;
struct Test_I_Source_DirectShow_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Source_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , builder (NULL)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , filterConfiguration (NULL)
   , filterIdentifier (GUID_NULL)
   , inputFormat (NULL)
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , sourceFormat (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
   , windowController2 (NULL)
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
    filterIdentifier = rhs_in.filterIdentifier;
    if (inputFormat)
      Stream_MediaFramework_DirectShow_Tools::delete_ (inputFormat);
    if (rhs_in.inputFormat)
    {
      inputFormat =
        Stream_MediaFramework_DirectShow_Tools::copy (*rhs_in.inputFormat);
      if (!inputFormat)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), returning\n")));
        return *this;
      } // end IF
    } // end IF
    push = rhs_in.push;
    if (sourceFormat)
      Stream_MediaFramework_DirectShow_Tools::delete_ (sourceFormat);
    if (rhs_in.sourceFormat)
    {
      sourceFormat =
        Stream_MediaFramework_DirectShow_Tools::copy (*rhs_in.sourceFormat);
      if (!sourceFormat)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), returning\n")));
        return *this;
      } // end IF
    } // end IF
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

  struct tagRECT                                       area; // visualization module
  IGraphBuilder*                                       builder;
  Test_I_Source_DirectShow_IConnection_t*              connection; // TCP target/IO module
  Test_I_Source_DirectShow_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_Source_DirectShow_InetConnectionManager_t*    connectionManager; // TCP IO module
  guint                                                contextId;
  struct Test_I_Source_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                filterIdentifier;
  struct _AMMediaType*                                 inputFormat; // source module
  bool                                                 push;
  struct _AMMediaType*                                 sourceFormat;
  Test_I_Source_DirectShow_StreamConfiguration_t*      streamConfiguration;
  Test_I_Source_DirectShow_ISessionNotify_t*           subscriber;
  Test_I_Source_DirectShow_Subscribers_t*              subscribers;
  IVideoWindow*                                        windowController; // visualization module
  IMFVideoDisplayControl*                              windowController2; // visualization module (EVR)
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_MediaFoundation_Stream_Message,
                                    Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_Source_MediaFoundation_ISessionNotify_t;
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
   , inputFormat (NULL)
   , mediaSource (NULL)
   , sampleGrabberNodeId (0)
   , session (NULL)
   , sourceFormat (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
  {
    finishOnDisconnect = true;

    HRESULT result = MFCreateMediaType (&inputFormat);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    ACE_ASSERT (inputFormat);
  }

  struct tagRECT                                            area;
  Test_I_Source_MediaFoundation_IConnection_t*              connection; // TCP target/IO module
  Test_I_Source_MediaFoundation_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_Source_MediaFoundation_InetConnectionManager_t*    connectionManager; // TCP IO module
  IMFMediaType*                                             inputFormat;
  TOPOID                                                    sampleGrabberNodeId;
  Test_I_Source_MediaFoundation_StreamConfiguration_t*      streamConfiguration;
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx*                                         mediaSource;
#else
  IMFMediaSource*                                           mediaSource;
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
  IMFMediaSession*                                          session;
  IMFMediaType*                                             sourceFormat;
  Test_I_Source_MediaFoundation_ISessionNotify_t*           subscriber;
  Test_I_Source_MediaFoundation_Subscribers_t*              subscribers;
  IMFVideoDisplayControl*                                   windowController;
};
#else
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_V4L2_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_Stream_SessionMessage> Test_I_Source_V4L2_ISessionNotify_t;
typedef std::list<Test_I_Source_V4L2_ISessionNotify_t*> Test_I_Source_V4L2_Subscribers_t;
typedef Test_I_Source_V4L2_Subscribers_t::iterator Test_I_Source_V4L2_SubscribersIterator_t;
struct Test_I_Source_V4L2_StreamConfiguration;
struct Test_I_Source_V4L2_ModuleHandlerConfiguration
 : Test_I_CamStream_ModuleHandlerConfiguration
{
  Test_I_Source_V4L2_ModuleHandlerConfiguration ()
   : Test_I_CamStream_ModuleHandlerConfiguration ()
   , area ()
   , buffers (MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , fileDescriptor (-1)
   , format (AV_PIX_FMT_RGB24)
   , sourceFormat ()
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , inputFormat ()
   , frameRate ()
   , v4l2Method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , v4l2Window (NULL)
   , userData (NULL)
  {
    finishOnDisconnect = true;

    ACE_OS::memset (&frameRate, 0, sizeof (struct v4l2_fract));
    ACE_OS::memset (&inputFormat, 0, sizeof (struct v4l2_format));
    inputFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  }

  GdkRectangle                                   area;
  __u32                                          buffers; // v4l device buffers
  Test_I_Source_V4L2_IConnection_t*              connection; // TCP target/IO module
  Test_I_Source_V4L2_ConnectionConfigurations_t* connectionConfigurations;
  Test_I_Source_V4L2_InetConnectionManager_t*    connectionManager; // TCP IO module
  int                                            fileDescriptor;
  enum AVPixelFormat                             format; // output-
  GdkRectangle                                   sourceFormat; // gtk pixbuf module
  ACE_Time_Value                                 statisticCollectionInterval;
  // *TODO*: remove this ASAP
  Test_I_Source_V4L2_StreamConfiguration_t*      streamConfiguration;
  Test_I_Source_V4L2_ISessionNotify_t*           subscriber;
  Test_I_Source_V4L2_Subscribers_t*              subscribers;
  struct v4l2_format                             inputFormat; // v4l2 camera source
  struct v4l2_fract                              frameRate; // v4l2 camera source
  enum v4l2_memory                               v4l2Method; // v4l2 camera source
  struct v4l2_window*                            v4l2Window; // v4l2 camera source

  struct Test_I_Source_V4L2_UserData*            userData;
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

  Test_I_Source_DirectShow_InetConnectionManager_t* connectionManager;
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

  Test_I_Source_MediaFoundation_InetConnectionManager_t* connectionManager;
  //  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_MediaFoundation_StreamBase_t*            stream;
};
typedef Test_I_Source_SignalHandler_T<struct Test_I_Source_MediaFoundation_SignalHandlerConfiguration> Test_I_Source_MediaFoundation_SignalHandler_t;
#else
struct Test_I_Source_V4L2_SignalHandlerConfiguration
 : Test_I_SignalHandlerConfiguration
{
  Test_I_Source_V4L2_SignalHandlerConfiguration ()
   : Test_I_SignalHandlerConfiguration ()
   , connectionManager (NULL)
   //   , statisticReportingInterval (0)
   , stream (NULL)
  {}

  Test_I_Source_V4L2_InetConnectionManager_t* connectionManager;
  //  unsigned int                statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_Source_V4L2_StreamBase_t*            stream;
};
typedef Test_I_Source_SignalHandler_T<struct Test_I_Source_V4L2_SignalHandlerConfiguration> Test_I_Source_V4L2_SignalHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_Source_Stream_StatisticData
 : Stream_Statistic
{
  Test_I_Source_Stream_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_I_Source_Stream_StatisticData operator+= (const struct Test_I_Source_Stream_StatisticData& rhs_in)
  {
    Stream_Statistic::operator+= (rhs_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    capturedFrames += rhs_in.capturedFrames;
#endif // ACE_WIN32 || ACE_WIN64

    return *this;
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  unsigned int capturedFrames;
#endif // ACE_WIN32 || ACE_WIN64
};
typedef Common_IStatistic_T<struct Test_I_Source_Stream_StatisticData> Test_I_Source_Stream_StatisticReportingHandler_t;
typedef Common_StatisticHandler_T<struct Test_I_Source_Stream_StatisticData> Test_I_Source_Stream_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_Source_DirectShow_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , graphLayout ()
   , userData (NULL)
  {}

  // **************************** stream data **********************************
  Stream_MediaFramework_DirectShow_Graph_t  graphLayout;

  struct Test_I_Source_DirectShow_UserData* userData;
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
 : Test_I_StreamConfiguration
{
  Test_I_Source_MediaFoundation_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , mediaFoundationConfiguration (NULL)
   , userData (NULL)
  {}

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration*    mediaFoundationConfiguration;
  // **************************** stream data **********************************

  struct Test_I_Source_MediaFoundation_UserData* userData;
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
struct Test_I_Source_V4L2_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_Source_V4L2_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , userData (NULL)
  {}

  // **************************** stream data **********************************
  struct Test_I_Source_V4L2_UserData* userData;
};

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AllocatorConfiguration,
                               struct Test_I_Source_V4L2_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_Source_V4L2_ModuleHandlerConfiguration> Test_I_Source_V4L2_StreamConfiguration_t;
typedef Test_I_Source_V4L2_StreamConfiguration_t::ITERATOR_T Test_I_Source_V4L2_StreamConfigurationIterator_t;
typedef std::map<std::string,
                 Test_I_Source_V4L2_StreamConfiguration_t> Test_I_Source_V4L2_StreamConfigurations_t;
typedef Test_I_Source_V4L2_StreamConfigurations_t::iterator Test_I_Source_V4L2_StreamConfigurationsIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_StreamState
 : Stream_State
{
  Test_I_Source_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  struct Test_I_Source_DirectShow_SessionData* sessionData;
  struct Test_I_Source_DirectShow_UserData*    userData;
};
struct Test_I_Source_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_Source_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  struct Test_I_Source_MediaFoundation_SessionData* sessionData;
  struct Test_I_Source_MediaFoundation_UserData*    userData;
};
#else
struct Test_I_Source_V4L2_StreamState
 : Stream_State
{
  Test_I_Source_V4L2_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  struct Test_I_Source_V4L2_SessionData* sessionData;

  struct Test_I_Source_V4L2_UserData*    userData;
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
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Test_I_Source_DirectShow_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_Source_DirectShow_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_Source_DirectShow_StreamConfigurations_t            streamConfigurations;

  struct Test_I_Source_DirectShow_UserData                   userData;
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
   , userData ()
  {}

  // **************************** media foundation *****************************
  struct Test_I_MediaFoundationConfiguration                      mediaFoundationConfiguration;
  // **************************** signal data **********************************
  struct Test_I_Source_MediaFoundation_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_Source_MediaFoundation_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_Source_MediaFoundation_StreamConfigurations_t            streamConfigurations;

  struct Test_I_Source_MediaFoundation_UserData                   userData;
};
#else
struct Test_I_Source_V4L2_Configuration
 : Test_I_CamStream_Configuration
{
  Test_I_Source_V4L2_Configuration ()
   : Test_I_CamStream_Configuration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfigurations ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Test_I_Source_V4L2_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_Source_V4L2_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_Source_V4L2_StreamConfigurations_t            streamConfigurations;

  struct Test_I_Source_V4L2_UserData                   userData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_ControlMessage_T<ENUM Stream_ControlMessageType,
//                                struct Test_I_AllocatorConfiguration> Test_I_DirectShow_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_Stream_SessionMessage> Test_I_Source_DirectShow_MessageAllocator_t;

struct Test_I_Source_DirectShow_UI_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Source_DirectShow_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_DirectShow_Stream_Message,
                                     Test_I_Source_DirectShow_Stream_SessionMessage,
                                     struct Test_I_Source_DirectShow_UI_CBData> Test_I_Source_DirectShow_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_Source_DirectShow_ISessionNotify_t> Test_I_Source_DirectShow_ISubscribe_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_MediaFoundation_Stream_Message,
                                          Test_I_Source_MediaFoundation_Stream_SessionMessage> Test_I_Source_MediaFoundation_MessageAllocator_t;

struct Test_I_Source_MediaFoundation_UI_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Source_MediaFoundation_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_MediaFoundation_Stream_Message,
                                     Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                     struct Test_I_Source_MediaFoundation_UI_CBData> Test_I_Source_MediaFoundation_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_Source_MediaFoundation_ISessionNotify_t> Test_I_Source_MediaFoundation_ISubscribe_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_V4L2_Stream_Message,
                                          Test_I_Source_V4L2_Stream_SessionMessage> Test_I_Source_V4L2_MessageAllocator_t;

struct Test_I_Source_V4L2_UI_CBData;
typedef Test_I_Source_EventHandler_T<Stream_SessionId_t,
                                     struct Test_I_Source_V4L2_SessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_Source_V4L2_Stream_Message,
                                     Test_I_Source_V4L2_Stream_SessionMessage,
                                     struct Test_I_Source_V4L2_UI_CBData> Test_I_Source_V4L2_EventHandler_t;

typedef Common_ISubscribe_T<Test_I_Source_V4L2_ISessionNotify_t> Test_I_Source_V4L2_ISubscribe_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

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
struct Test_I_Source_V4L2_UI_CBData
 : Test_I_CamStream_UI_CBData
{
  Test_I_Source_V4L2_UI_CBData ()
   : Test_I_CamStream_UI_CBData ()
   , configuration (NULL)
   , fileDescriptor (-1)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
   , UDPStream (NULL)
  {}

  struct Test_I_Source_V4L2_Configuration* configuration;
  int                                      fileDescriptor; // (capture) device file descriptor
  Test_I_Source_V4L2_StreamBase_t*         stream;
  Test_I_Source_V4L2_Subscribers_t         subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX                subscribersLock;
  Test_I_Source_V4L2_StreamBase_t*         UDPStream;
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
struct Test_I_Source_V4L2_ThreadData
 : Test_I_CamStream_ThreadData
{
  Test_I_Source_V4L2_ThreadData ()
   : Test_I_CamStream_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_Source_V4L2_UI_CBData* CBData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct Test_I_Source_DirectShow_UI_CBData> Test_I_Source_DirectShow_GtkBuilderDefinition_t;
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct Test_I_Source_MediaFoundation_UI_CBData> Test_I_Source_MediaFoundation_GtkBuilderDefinition_t;
#else
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct Test_I_GTK_CBData> Test_I_Source_GtkBuilderDefinition_t;
#endif // ACE_WIN32 || ACE_WIN64
#endif // GTK_SUPPORT

#endif
