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

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_mediafoundation_callback.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_net_io_stream.h"
#include "stream_net_target.h"

#include "test_i_camstream_network.h"
#include "test_i_connection_manager_common.h"
#include "test_i_source_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMFMediaSession;
#endif // ACE_WIN32 || ACE_WIN64
class Stream_IAllocator;

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename StreamStateType,
          typename ConfigurationType,
          typename TimerManagerType, // implements Common_ITimer
          typename HandlerConfigurationType, // module-
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
class Test_I_Source_DirectShow_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        struct Test_I_Source_Stream_StatisticData,
                        struct Test_I_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        struct Test_I_Source_Stream_StatisticData,
                        struct Test_I_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType> inherited;

 public:
  Test_I_Source_DirectShow_Stream_T ();
  virtual ~Test_I_Source_DirectShow_Stream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&);

 private:
  typedef Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                            ConfigurationType,
                                            TimerManagerType,
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
                                     Test_I_Source_DirectShow_ConnectionConfigurationIterator_t,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,               // task synch type
                                         Common_TimePolicy_t,        // time policy
                                         Stream_SessionId_t,         // session id type
                                         SessionDataType,            // session data type
                                         Stream_SessionMessageType,  // session event type
                                         Stream_ModuleConfiguration, // module configuration type
                                         HandlerConfigurationType,   // module handler configuration type
                                         libacestream_default_net_target_module_name_string,
                                         Stream_INotify_t,           // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;  // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_T (const Test_I_Source_DirectShow_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_DirectShow_Stream_T& operator= (const Test_I_Source_DirectShow_Stream_T&))
};

//////////////////////////////////////////

template <typename StreamStateType,
          typename ConfigurationType,
          typename TimerManagerType, // implements Common_ITimer
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
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        struct Test_I_Source_Stream_StatisticData,
                        struct Test_I_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType>
 , public Stream_MediaFramework_MediaFoundation_Callback_T<struct Test_I_MediaFoundationConfiguration>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        struct Test_I_Source_Stream_StatisticData,
                        struct Test_I_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType> inherited;
  typedef Stream_MediaFramework_MediaFoundation_Callback_T<struct Test_I_MediaFoundationConfiguration> inherited2;

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

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&);

 private:
  typedef Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                                 ConfigurationType,
                                                 TimerManagerType,
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
                                     Test_I_Source_MediaFoundation_ConnectionConfigurationIterator_t,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,               // task synch type
                                         Common_TimePolicy_t,        // time policy
                                         Stream_SessionId_t,         // session id type
                                         SessionDataType,            // session data type
                                         Stream_SessionMessageType,  // session event type
                                         Stream_ModuleConfiguration, // module configuration type
                                         HandlerConfigurationType,   // module handler configuration type
                                         libacestream_default_net_target_module_name_string,
                                         Stream_INotify_t,     // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;  // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_T (const Test_I_Source_MediaFoundation_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_MediaFoundation_Stream_T& operator= (const Test_I_Source_MediaFoundation_Stream_T&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // media session
  IMFMediaSession* mediaSession_;
};
#else
template <typename StreamStateType,
          typename ConfigurationType,
          typename TimerManagerType, // implements Common_ITimer
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
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        struct Test_I_Source_V4L2_StreamConfiguration,
                        struct Test_I_Source_Stream_StatisticData,
                        struct Test_I_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        struct Test_I_Source_V4L2_StreamConfiguration,
                        struct Test_I_Source_Stream_StatisticData,
                        struct Test_I_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        MessageType,
                        SessionMessageType> inherited;

 public:
  Test_I_Source_V4L2_Stream_T ();
  virtual ~Test_I_Source_V4L2_Stream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  typedef Test_I_Source_V4L2_Stream_T<StreamStateType,
                                      ConfigurationType,
                                      TimerManagerType,
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
                                     Test_I_Source_V4L2_ConnectionConfigurationIterator_t,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                      // task synch type
                                         Common_TimePolicy_t,               // time policy
                                         Stream_SessionId_t,                // session id type
                                         SessionDataType,                   // session data type
                                         enum Stream_SessionMessageType,    // session event type
                                         struct Stream_ModuleConfiguration, // module configuration type
                                         HandlerConfigurationType,          // module handler configuration type
                                         libacestream_default_net_target_module_name_string,
                                         Stream_INotify_t,            // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;         // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L2_Stream_T (const Test_I_Source_V4L2_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_V4L2_Stream_T& operator= (const Test_I_Source_V4L2_Stream_T&))

  // *TODO*: re-consider this API
  void ping ();
};
#endif // ACE_WIN32 || ACE_WIN64

// include template definition
#include "test_i_source_stream.inl"

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
                                          Common_Timer_Manager_t,
                                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_TCPConnector_t> Test_I_Source_DirectShow_TCPStream_t;
#if defined (SSL_SUPPORT)
typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
                                          Common_Timer_Manager_t,
                                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_SSLTCPConnector_t> Test_I_Source_DirectShow_SSLTCPStream_t;
#endif // SSL_SUPPORT
typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
                                          Common_Timer_Manager_t,
                                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_TCPAsynchConnector_t> Test_I_Source_DirectShow_AsynchTCPStream_t;

typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
                                          Common_Timer_Manager_t,
                                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_UDPConnector_t> Test_I_Source_DirectShow_UDPStream_t;
typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
                                          Common_Timer_Manager_t,
                                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_UDPAsynchConnector_t> Test_I_Source_DirectShow_AsynchUDPStream_t;

typedef Test_I_Source_MediaFoundation_Stream_T<struct Test_I_Source_MediaFoundation_StreamState,
                                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Common_Timer_Manager_t,
                                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_TCPConnector_t> Test_I_Source_MediaFoundation_TCPStream_t;
#if defined (SSL_SUPPORT)
typedef Test_I_Source_MediaFoundation_Stream_T<struct Test_I_Source_MediaFoundation_StreamState,
                                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Common_Timer_Manager_t,
                                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_SSLTCPConnector_t> Test_I_Source_MediaFoundation_SSLTCPStream_t;
#endif // SSL_SUPPORT
typedef Test_I_Source_MediaFoundation_Stream_T<struct Test_I_Source_MediaFoundation_StreamState,
                                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Common_Timer_Manager_t,
                                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_TCPAsynchConnector_t> Test_I_Source_MediaFoundation_AsynchTCPStream_t;

typedef Test_I_Source_MediaFoundation_Stream_T<struct Test_I_Source_MediaFoundation_StreamState,
                                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Common_Timer_Manager_t,
                                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_UDPConnector_t> Test_I_Source_MediaFoundation_UDPStream_t;
typedef Test_I_Source_MediaFoundation_Stream_T<struct Test_I_Source_MediaFoundation_StreamState,
                                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
                                               Common_Timer_Manager_t,
                                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_UDPAsynchConnector_t> Test_I_Source_MediaFoundation_AsynchUDPStream_t;
#else
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration_t,
                                    Common_Timer_Manager_t,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_TCPConnector_t> Test_I_Source_V4L2_TCPStream_t;
#if defined (SSL_SUPPORT)
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration_t,
                                    Common_Timer_Manager_t,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_SSLTCPConnector_t> Test_I_Source_V4L2_SSLTCPStream_t;
#endif // SSL_SUPPORT
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration_t,
                                    Common_Timer_Manager_t,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_TCPAsynchConnector_t> Test_I_Source_V4L2_AsynchTCPStream_t;

typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration_t,
                                    Common_Timer_Manager_t,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_UDPConnector_t> Test_I_Source_V4L2_UDPStream_t;
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    Test_I_Source_V4L2_StreamConfiguration_t,
                                    Common_Timer_Manager_t,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_UDPAsynchConnector_t> Test_I_Source_V4L2_AsynchUDPStream_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
