/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef TEST_I_SMTP_SEND_NETWORK_H
#define TEST_I_SMTP_SEND_NETWORK_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "smtp_common.h"
#include "smtp_configuration.h"
#include "smtp_stream_common.h"
#include "smtp_network.h"

#include "test_i_smtp_send_stream_common.h"

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                SMTP_ConnectionConfiguration,
                                struct SMTP_ConnectionState,
                                struct Net_StreamStatistic,
                                Net_TCPSocketConfiguration_t,
                                Test_I_SMTPSend_Stream_t,
                                enum Stream_StateMachine_ControlState> Test_I_SMTPSend_IStreamConnection_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                SMTP_ConnectionConfiguration,
                                struct SMTP_ConnectionState,
                                struct Net_StreamStatistic,
                                Test_I_SMTPSend_Stream_t,
                                struct Net_UserData> Test_I_SMTPSend_Connection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                SMTP_ConnectionConfiguration,
                                struct SMTP_ConnectionState,
                                struct Net_StreamStatistic,
                                Test_I_SMTPSend_Stream_t,
                                struct Net_UserData> Test_I_SMTPSend_SSLConnection_t;
#endif // SSL_SUPPORT

typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      SMTP_ConnectionConfiguration,
                                      struct SMTP_ConnectionState,
                                      struct Net_StreamStatistic,
                                      Test_I_SMTPSend_Stream_t,
                                      struct Net_UserData> Test_I_SMTPSend_AsynchConnection_t;

/////////////////////////////////////////

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               SMTP_Connection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               SMTP_ConnectionConfiguration,
                               struct SMTP_ConnectionState,
                               struct Net_StreamStatistic,
                               Net_TCPSocketConfiguration_t,
                               Test_I_SMTPSend_Stream_t,
                               struct Net_UserData> Test_I_SMTPSend_Connector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<SMTP_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   SMTP_ConnectionConfiguration,
                                   struct SMTP_ConnectionState,
                                   struct Net_StreamStatistic,
                                   Test_I_SMTPSend_Stream_t,
                                   struct Net_UserData> Test_I_SMTPSend_SSLConnector_t;
#endif // SSL_SUPPORT

typedef Net_Client_AsynchConnector_T<SMTP_AsynchConnection_t,
                                     ACE_INET_Addr,
                                     SMTP_ConnectionConfiguration,
                                     struct SMTP_ConnectionState,
                                     struct Net_StreamStatistic,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_SMTPSend_Stream_t,
                                     struct Net_UserData> Test_I_SMTPSend_AsynchConnector_t;

/////////////////////////////////////////

#endif
