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

#ifndef TEST_I_POP_RECEIVE_COMMON_MODULES_H
#define TEST_I_POP_RECEIVE_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_defines.h"
#include "stream_misc_messagehandler.h"

#include "stream_net_source.h"

#include "pop_common.h"
#include "pop_module_send.h"
#include "pop_stream_common.h"

#include "test_i_pop_receive_common.h"
#include "test_i_pop_receive_network.h"

typedef Stream_Module_Net_Source_Reader_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          struct Stream_POPReceive_ModuleHandlerConfiguration,
                                          POP_SessionManager_t,
                                          Stream_ControlMessage_t,
                                          POP_Message_t,
                                          POP_SessionMessage_t> Stream_POPReceive_NetSourceReader;

typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    POP_Message_t,
                                    POP_SessionMessage_t,
                                    struct Stream_POPReceive_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct POP_StreamState,
                                    POP_Statistic_t,
                                    POP_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    Test_I_POPReceive_Connector_t,
                                    struct Stream_UserData> Stream_POPReceive_NetSource;
typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    POP_Message_t,
                                    POP_SessionMessage_t,
                                    struct Stream_POPReceive_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct POP_StreamState,
                                    POP_Statistic_t,
                                    POP_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    Test_I_POPReceive_AsynchConnector_t,
                                    struct Stream_UserData> Stream_POPReceive_AsynchNetSource;
#if defined (SSL_SUPPORT)
typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    POP_Message_t,
                                    POP_SessionMessage_t,
                                    struct Stream_POPReceive_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct POP_StreamState,
                                    POP_Statistic_t,
                                    POP_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    Test_I_POPReceive_SSLConnector_t,
                                    struct Stream_UserData> Stream_POPReceive_SSLNetSource;
#endif // SSL_SUPPORT

typedef POP_Module_Send_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           Stream_ControlMessage_t,
                           POP_Message_t,
                           POP_SessionMessage_t,
                           struct Stream_POPReceive_ModuleHandlerConfiguration,
                           struct POP_ConnectionState> Stream_POPReceive_ProtocolHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_POPReceive_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       POP_Message_t,
                                       POP_SessionMessage_t,
                                       struct POP_Stream_SessionData,
                                       struct Stream_UserData> Stream_POPReceive_MessageHandler;

//////////////////////////////////////////
// declare module(s)
DATASTREAM_MODULE_DUPLEX (struct POP_Stream_SessionData,                    // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_POPReceive_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_source_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_POPReceive_NetSourceReader,                  // reader type
                          Stream_POPReceive_NetSource,                        // writer type
                          Stream_POPReceive_NetSource);                       // name
DATASTREAM_MODULE_DUPLEX (struct POP_Stream_SessionData,                    // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_POPReceive_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_source_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_POPReceive_NetSourceReader,                  // reader type
                          Stream_POPReceive_AsynchNetSource,                  // writer type
                          Stream_POPReceive_AsynchNetSource);                 // name
#if defined (SSL_SUPPORT)
DATASTREAM_MODULE_DUPLEX (struct POP_Stream_SessionData,                    // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_POPReceive_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_source_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_POPReceive_NetSourceReader,                  // reader type
                          Stream_POPReceive_SSLNetSource,                     // writer type
                          Stream_POPReceive_SSLNetSource);                    // name
#endif // SSL_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (struct POP_Stream_SessionData,                    // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Stream_POPReceive_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_protocol_default_pop_send_module_name_string,
                              Stream_INotify_t,                                  // stream notification interface type
                              Stream_POPReceive_ProtocolHandler);                  // writer type

DATASTREAM_MODULE_INPUT_ONLY (struct POP_Stream_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_POPReceive_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_POPReceive_MessageHandler);                      // writer type

#endif
