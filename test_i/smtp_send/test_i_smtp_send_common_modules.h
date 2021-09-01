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

#ifndef TEST_I_SMTPSEND_COMMON_MODULES_H
#define TEST_I_SMTPSEND_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_defines.h"
#include "stream_misc_messagehandler.h"

#include "stream_net_source.h"

#include "smtp_common.h"
#include "smtp_module_send.h"
#include "smtp_stream_common.h"

#include "test_i_smtp_send_common.h"
#include "test_i_smtp_send_network.h"

typedef Stream_Module_Net_Source_Reader_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          struct Stream_SMTPSend_ModuleHandlerConfiguration,
                                          Stream_ControlMessage_t,
                                          SMTP_Message_t,
                                          SMTP_SessionMessage_t> Stream_SMTPSend_NetSourceReader;

typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    SMTP_Message_t,
                                    SMTP_SessionMessage_t,
                                    struct Stream_SMTPSend_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct SMTP_StreamState,
                                    SMTP_Statistic_t,
                                    Common_Timer_Manager_t,
                                    Test_I_SMTPSend_Connector_t,
                                    struct Stream_UserData> Stream_SMTPSend_NetSource;
typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    SMTP_Message_t,
                                    SMTP_SessionMessage_t,
                                    struct Stream_SMTPSend_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct SMTP_StreamState,
                                    SMTP_Statistic_t,
                                    Common_Timer_Manager_t,
                                    Test_I_SMTPSend_AsynchConnector_t,
                                    struct Stream_UserData> Stream_SMTPSend_AsynchNetSource;
#if defined (SSL_SUPPORT)
typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    SMTP_Message_t,
                                    SMTP_SessionMessage_t,
                                    struct Stream_SMTPSend_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct SMTP_StreamState,
                                    SMTP_Statistic_t,
                                    Common_Timer_Manager_t,
                                    Test_I_SMTPSend_SSLConnector_t,
                                    struct Stream_UserData> Stream_SMTPSend_SSLNetSource;
#endif // SSL_SUPPORT

typedef SMTP_Module_Send_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           Stream_ControlMessage_t,
                           SMTP_Message_t,
                           SMTP_SessionMessage_t,
                           struct Stream_SMTPSend_ModuleHandlerConfiguration,
                           struct SMTP_ConnectionState> Stream_SMTPSend_ProtocolHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_SMTPSend_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       SMTP_Message_t,
                                       SMTP_SessionMessage_t,
                                       struct SMTP_Stream_SessionData,
                                       struct Stream_UserData> Stream_SMTPSend_MessageHandler;

//////////////////////////////////////////
// declare module(s)
DATASTREAM_MODULE_DUPLEX (struct SMTP_Stream_SessionData,                    // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_SMTPSend_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_source_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_SMTPSend_NetSourceReader,                  // reader type
                          Stream_SMTPSend_NetSource,                        // writer type
                          Stream_SMTPSend_NetSource);                       // name
DATASTREAM_MODULE_DUPLEX (struct SMTP_Stream_SessionData,                    // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_SMTPSend_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_source_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_SMTPSend_NetSourceReader,                  // reader type
                          Stream_SMTPSend_AsynchNetSource,                  // writer type
                          Stream_SMTPSend_AsynchNetSource);                 // name
#if defined (SSL_SUPPORT)
DATASTREAM_MODULE_DUPLEX (struct SMTP_Stream_SessionData,                    // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_SMTPSend_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_source_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_SMTPSend_NetSourceReader,                  // reader type
                          Stream_SMTPSend_SSLNetSource,                     // writer type
                          Stream_SMTPSend_SSLNetSource);                    // name
#endif // SSL_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (struct SMTP_Stream_SessionData,                    // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Stream_SMTPSend_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_protocol_default_smtp_send_module_name_string,
                              Stream_INotify_t,                                  // stream notification interface type
                              Stream_SMTPSend_ProtocolHandler);                  // writer type

DATASTREAM_MODULE_INPUT_ONLY (struct SMTP_Stream_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_SMTPSend_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_SMTPSend_MessageHandler);                      // writer type

#endif
