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

#ifndef TEST_I_SOURCE_STREAM_H
#define TEST_I_SOURCE_STREAM_H

#include <string>

#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_misc_mediafoundation_callback.h"
#endif

#include "stream_module_target.h"

#include "test_i_source_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMFMediaSession;
#endif
class Stream_IAllocator;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
class Test_I_Source_DirectShow_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType>
{
 public:
  Test_I_Source_DirectShow_Stream_T ();
  virtual ~Test_I_Source_DirectShow_Stream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&, // configuration
                           bool = true,              // setup pipeline ?
                           bool = true);             // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_Source_Stream_StatisticData&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType> inherited;

  typedef Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                            ConfigurationType,
                                            HandlerConfigurationType,
                                            SessionDataType,
                                            SessionDataContainerType,
                                            ControlMessageType,
                                            MessageType,
                                            SessionMessageType,
                                            ConnectionManagerType,
                                            ConnectorType> OWN_TYPE_T;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     HandlerConfigurationType,
                                     ControlMessageType,
                                     MessageType,
                                     SessionMessageType,
                                     SessionDataContainerType,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,               // task synch type
                                         Common_TimePolicy_t,        // time policy
                                         Stream_SessionId_t,         // session id type
                                         SessionDataType,            // session data type
                                         Stream_SessionMessageType,  // session event type
                                         Stream_ModuleConfiguration, // module configuration type
                                         HandlerConfigurationType,   // module handler configuration type
                                         Test_I_IStreamNotify_t,     // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;  // writer type

  //ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_T (const Test_I_Source_DirectShow_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_T& operator= (const Test_I_Source_DirectShow_Stream_T&))

  // *TODO*: re-consider this API
  void ping ();

  IGraphBuilder* graphBuilder_;
};

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
class Test_I_Source_MediaFoundation_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType>
 , public Stream_Misc_MediaFoundation_Callback_T<Test_I_MediaFoundationConfiguration>
{
 public:
  Test_I_Source_MediaFoundation_Stream_T ();
  virtual ~Test_I_Source_MediaFoundation_Stream_T ();

  // override (part of) Stream_IStreamControl_T
  //virtual Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&, // configuration
                           bool = true,              // setup pipeline ?
                           bool = true);             // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_Source_Stream_StatisticData&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType> inherited;
  typedef Stream_Misc_MediaFoundation_Callback_T<Test_I_MediaFoundationConfiguration> inherited2;

  typedef Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                                 ConfigurationType,
                                                 HandlerConfigurationType,
                                                 SessionDataType,
                                                 SessionDataContainerType,
                                                 ControlMessageType,
                                                 MessageType,
                                                 SessionMessageType,
                                                 ConnectionManagerType,
                                                 ConnectorType> OWN_TYPE_T;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     HandlerConfigurationType,
                                     ControlMessageType,
                                     MessageType,
                                     SessionMessageType,
                                     SessionDataContainerType,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,               // task synch type
                                         Common_TimePolicy_t,        // time policy
                                         Stream_SessionId_t,         // session id type
                                         SessionDataType,            // session data type
                                         Stream_SessionMessageType,  // session event type
                                         Stream_ModuleConfiguration, // module configuration type
                                         HandlerConfigurationType,   // module handler configuration type
                                         Test_I_IStreamNotify_t,     // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;  // writer type

  //ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_T (const Test_I_Source_MediaFoundation_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_T& operator= (const Test_I_Source_MediaFoundation_Stream_T&))

  // *TODO*: re-consider this API
  void ping ();

  // media session
  IMFMediaSession* mediaSession_;
};
#else
template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
class Test_I_Source_V4L2_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType>
{
 public:
  Test_I_Source_V4L2_Stream_T ();
  virtual ~Test_I_Source_V4L2_Stream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&, // configuration
                           bool = true,              // setup pipeline ?
                           bool = true);             // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_Source_Stream_StatisticData&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType> inherited;

  typedef Test_I_Source_V4L2_Stream_T<StreamStateType,
                                      ConfigurationType,
                                      HandlerConfigurationType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      ControlMessageType,
                                      MessageType,
                                      SessionMessageType,
                                      ConnectionManagerType,
                                      ConnectorType> OWN_TYPE_T;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     HandlerConfigurationType,
                                     ControlMessageType,
                                     MessageType,
                                     SessionMessageType,
                                     SessionDataContainerType,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,               // task synch type
                                         Common_TimePolicy_t,        // time policy
                                         Stream_SessionId_t,         // session id type
                                         SessionDataType,            // session data type
                                         Stream_SessionMessageType,  // session event type
                                         Stream_ModuleConfiguration, // module configuration type
                                         HandlerConfigurationType,   // module handler configuration type
                                         Test_I_IStreamNotify_t,     // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;  // writer type

  //ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L2_Stream_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L2_Stream_T (const Test_I_Source_V4L2_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L2_Stream_T& operator= (const Test_I_Source_V4L2_Stream_T&))

  // *TODO*: re-consider this API
  void ping ();
};
#endif

// include template definition
#include "test_i_source_stream.inl"

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Source_DirectShow_Stream_T<Test_I_Source_DirectShow_StreamState,
                                          Test_I_Source_DirectShow_StreamConfiguration,
                                          Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_DirectShow_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_Stream_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_TCPConnector_t> Test_I_Source_DirectShow_TCPStream_t;
typedef Test_I_Source_DirectShow_Stream_T<Test_I_Source_DirectShow_StreamState,
                                          Test_I_Source_DirectShow_StreamConfiguration,
                                          Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_DirectShow_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_Stream_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_SSLTCPConnector_t> Test_I_Source_DirectShow_SSLTCPStream_t;
typedef Test_I_Source_DirectShow_Stream_T<Test_I_Source_DirectShow_StreamState,
                                          Test_I_Source_DirectShow_StreamConfiguration,
                                          Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_DirectShow_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_Stream_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_TCPAsynchConnector_t> Test_I_Source_DirectShow_AsynchTCPStream_t;
typedef Test_I_Source_DirectShow_Stream_T<Test_I_Source_DirectShow_StreamState,
                                          Test_I_Source_DirectShow_StreamConfiguration,
                                          Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_DirectShow_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_Stream_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_UDPConnector_t> Test_I_Source_DirectShow_UDPStream_t;
typedef Test_I_Source_DirectShow_Stream_T<Test_I_Source_DirectShow_StreamState,
                                          Test_I_Source_DirectShow_StreamConfiguration,
                                          Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_DirectShow_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_Stream_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_UDPAsynchConnector_t> Test_I_Source_DirectShow_AsynchUDPStream_t;

typedef Test_I_Source_MediaFoundation_Stream_T<Test_I_Source_MediaFoundation_StreamState,
                                               Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_MediaFoundation_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_TCPConnector_t> Test_I_Source_MediaFoundation_TCPStream_t;
typedef Test_I_Source_MediaFoundation_Stream_T<Test_I_Source_MediaFoundation_StreamState,
                                               Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_MediaFoundation_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_SSLTCPConnector_t> Test_I_Source_MediaFoundation_SSLTCPStream_t;
typedef Test_I_Source_MediaFoundation_Stream_T<Test_I_Source_MediaFoundation_StreamState,
                                               Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_MediaFoundation_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_TCPAsynchConnector_t> Test_I_Source_MediaFoundation_AsynchTCPStream_t;
typedef Test_I_Source_MediaFoundation_Stream_T<Test_I_Source_MediaFoundation_StreamState,
                                               Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_MediaFoundation_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_UDPConnector_t> Test_I_Source_MediaFoundation_UDPStream_t;
typedef Test_I_Source_MediaFoundation_Stream_T<Test_I_Source_MediaFoundation_StreamState,
                                               Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_MediaFoundation_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_UDPAsynchConnector_t> Test_I_Source_MediaFoundation_AsynchUDPStream_t;
#else
typedef Test_I_Source_V4L2_Stream_T<Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration,
                                    Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_V4L2_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_Stream_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_TCPConnector_t> Test_I_Source_V4L2_TCPStream_t;
typedef Test_I_Source_V4L2_Stream_T<Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration,
                                    Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_V4L2_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_Stream_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_SSLTCPConnector_t> Test_I_Source_V4L2_SSLTCPStream_t;
typedef Test_I_Source_V4L2_Stream_T<Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration,
                                    Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_V4L2_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_Stream_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_TCPAsynchConnector_t> Test_I_Source_V4L2_AsynchTCPStream_t;
typedef Test_I_Source_V4L2_Stream_T<Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration,
                                    Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_V4L2_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_Stream_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_UDPConnector_t> Test_I_Source_V4L2_UDPStream_t;
typedef Test_I_Source_V4L2_Stream_T<Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration,
                                    Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_V4L2_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_Stream_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_UDPAsynchConnector_t> Test_I_Source_V4L2_AsynchUDPStream_t;
#endif

#endif
