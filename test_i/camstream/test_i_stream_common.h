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

#ifndef TEST_I_STREAM_COMMON_H
#define TEST_I_STREAM_COMMON_H

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_source_common.h"
#include "test_i_source_stream.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
                                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_TCPConnector_t> Test_I_Source_DirectShow_TCPStream_t;
typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
                                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                          Test_I_Source_DirectShow_SessionData,
                                          Test_I_Source_DirectShow_SessionData_t,
                                          Test_I_ControlMessage_t,
                                          Test_I_Source_DirectShow_Stream_Message,
                                          Test_I_Source_DirectShow_SessionMessage,
                                          Test_I_Source_DirectShow_InetConnectionManager_t,
                                          Test_I_Source_DirectShow_SSLTCPConnector_t> Test_I_Source_DirectShow_SSLTCPStream_t;
typedef Test_I_Source_DirectShow_Stream_T<struct Test_I_Source_DirectShow_StreamState,
                                          struct Test_I_Source_DirectShow_StreamConfiguration,
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
                                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_TCPConnector_t> Test_I_Source_MediaFoundation_TCPStream_t;
typedef Test_I_Source_MediaFoundation_Stream_T<struct Test_I_Source_MediaFoundation_StreamState,
                                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
                                               struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_I_Source_MediaFoundation_SessionData,
                                               Test_I_Source_MediaFoundation_SessionData_t,
                                               Test_I_ControlMessage_t,
                                               Test_I_Source_MediaFoundation_Stream_Message,
                                               Test_I_Source_MediaFoundation_SessionMessage,
                                               Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                               Test_I_Source_MediaFoundation_SSLTCPConnector_t> Test_I_Source_MediaFoundation_SSLTCPStream_t;
typedef Test_I_Source_MediaFoundation_Stream_T<struct Test_I_Source_MediaFoundation_StreamState,
                                               struct Test_I_Source_MediaFoundation_StreamConfiguration,
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
                                    struct Test_I_Source_V4L2_StreamConfiguration,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_TCPConnector_t> Test_I_Source_V4L2_TCPStream_t;
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    struct Test_I_Source_V4L2_StreamConfiguration,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_SSLTCPConnector_t> Test_I_Source_V4L2_SSLTCPStream_t;
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    struct Test_I_Source_V4L2_StreamConfiguration,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_TCPAsynchConnector_t> Test_I_Source_V4L2_AsynchTCPStream_t;
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    struct Test_I_Source_V4L2_StreamConfiguration,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_UDPConnector_t> Test_I_Source_V4L2_UDPStream_t;
typedef Test_I_Source_V4L2_Stream_T<struct Test_I_Source_V4L2_StreamState,
                                    struct Test_I_Source_V4L2_StreamConfiguration,
                                    struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                    Test_I_Source_V4L2_SessionData,
                                    Test_I_Source_V4L2_SessionData_t,
                                    Test_I_ControlMessage_t,
                                    Test_I_Source_V4L2_Stream_Message,
                                    Test_I_Source_V4L2_SessionMessage,
                                    Test_I_Source_V4L2_InetConnectionManager_t,
                                    Test_I_Source_V4L2_UDPAsynchConnector_t> Test_I_Source_V4L2_AsynchUDPStream_t;
#endif

#endif
